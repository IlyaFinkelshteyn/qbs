/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qbs.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "artifact.h"

#include "transformer.h"
#include "buildgraphvisitor.h"
#include "productbuilddata.h"
#include <language/language.h>
#include <language/propertymapinternal.h>
#include <tools/fileinfo.h>
#include <tools/persistence.h>

QT_BEGIN_NAMESPACE

static QDataStream &operator >>(QDataStream &s, qbs::Internal::Artifact::ArtifactType &t)
{
    int i;
    s >> i;
    t = static_cast<qbs::Internal::Artifact::ArtifactType>(i);
    return s;
}

static QDataStream &operator <<(QDataStream &s, const qbs::Internal::Artifact::ArtifactType &t)
{
    return s << (int)t;
}

QT_END_NAMESPACE

namespace qbs {
namespace Internal {

Artifact::Artifact()
{
    initialize();
}

Artifact::~Artifact()
{
    for (Artifact *p : parentArtifacts())
        p->childrenAddedByScanner.remove(this);
}

void Artifact::accept(BuildGraphVisitor *visitor)
{
    if (visitor->visit(this))
        acceptChildren(visitor);
    visitor->endVisit(this);
}

QString Artifact::toString() const
{
    return QLatin1String("ARTIFACT ") + filePath() + QLatin1String(" [")
            + (!product.isNull() ? product->name : QLatin1String("<null>")) + QLatin1Char(']');
}

void Artifact::addFileTag(const FileTag &t)
{
    m_fileTags += t;
    if (!product.isNull() && product->buildData)
        product->buildData->artifactsByFileTag[t] += this;
}

void Artifact::removeFileTag(const FileTag &t)
{
    m_fileTags -= t;
    if (!product.isNull() && product->buildData)
        removeArtifactFromSetByFileTag(this, t, product->buildData->artifactsByFileTag);
}

void Artifact::setFileTags(const FileTags &newFileTags)
{
    if (product.isNull() || !product->buildData) {
        m_fileTags = newFileTags;
        return;
    }
    foreach (const FileTag &t, m_fileTags)
        removeArtifactFromSetByFileTag(this, t, product->buildData->artifactsByFileTag);
    m_fileTags = newFileTags;
    addArtifactToSet(this, product->buildData->artifactsByFileTag);
}

void Artifact::initialize()
{
    artifactType = Unknown;
    inputsScanned = false;
    timestampRetrieved = false;
    alwaysUpdated = true;
    oldDataPossiblyPresent = true;
}

const TypeFilter<Artifact> Artifact::parentArtifacts() const
{
    return TypeFilter<Artifact>(parents);
}

const TypeFilter<Artifact> Artifact::childArtifacts() const
{
    return TypeFilter<Artifact>(children);
}

void Artifact::onChildDisconnected(BuildGraphNode *child)
{
    Artifact *childArtifact = dynamic_cast<Artifact *>(child);
    if (!childArtifact)
        return;
    childrenAddedByScanner.remove(childArtifact);
}

void Artifact::load(PersistentPool &pool)
{
    FileResourceBase::load(pool);
    BuildGraphNode::load(pool);
    children.load(pool);

    // restore parents of the loaded children
    for (NodeSet::const_iterator it = children.constBegin(); it != children.constEnd(); ++it)
        (*it)->parents.insert(this);

    pool.loadContainer(childrenAddedByScanner);
    pool.loadContainer(fileDependencies);
    properties = pool.idLoadS<PropertyMapInternal>();
    transformer = pool.idLoadS<Transformer>();
    m_fileTags.load(pool);
    unsigned char c;
    pool.stream()
            >> artifactType
            >> c;
    alwaysUpdated = c;
    pool.stream() >> c;
    oldDataPossiblyPresent = c;
}

void Artifact::store(PersistentPool &pool) const
{
    FileResourceBase::store(pool);
    BuildGraphNode::store(pool);
    // Do not store parents to avoid recursion.
    children.store(pool);
    pool.storeContainer(childrenAddedByScanner);
    pool.storeContainer(fileDependencies);
    pool.store(properties);
    pool.store(transformer);
    m_fileTags.store(pool);
    pool.stream()
            << artifactType
            << static_cast<unsigned char>(alwaysUpdated)
            << static_cast<unsigned char>(oldDataPossiblyPresent);
}

} // namespace Internal
} // namespace qbs

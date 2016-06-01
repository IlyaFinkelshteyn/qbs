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

#include "pbxfilereference.h"
#include "pbxgroup.h"

class PBXGroupPrivate
{
public:
    QList<PBXReference *> children;
};

PBXGroup::PBXGroup(QObject *parent) :
    PBXReference(parent), d(new PBXGroupPrivate)
{
}

PBXGroup::~PBXGroup()
{
    delete d;
}

PBXGroup *PBXGroup::groupWithName(const QString &name, PBXGroup *parentGroup)
{
    PBXGroup *group = new PBXGroup();
    group->setName(name);

    if (parentGroup) {
        group->setParent(parentGroup);
        parentGroup->addItem(group);
    }

    return group;
}

QString PBXGroup::isa() const
{
    return QStringLiteral("PBXGroup");
}

PBXObjectMap PBXGroup::toMap() const
{
    PBXObjectMap self = PBXReference::toMap();

    QVariantList childIdentifiers;
    for (PBXReference *reference : d->children)
        childIdentifiers += QVariant::fromValue(reference->identifier());

    self.insert(QStringLiteral("children"), childIdentifiers);

    return self;
}

QList<PBXReference *> PBXGroup::children() const
{
    return d->children;
}

void PBXGroup::addItem(PBXReference *item)
{
    d->children.append(item);
}

void PBXGroup::insertItem(PBXReference *item, int index)
{
    d->children.insert(index, item);
}

void PBXGroup::removeItem(PBXReference *item)
{
    d->children.removeOne(item);
}

QList<PBXFileReference *> PBXGroup::addFiles(const QStringList &files)
{
    QList<PBXFileReference *> fileRefs;
    for (const QString &filePath : files) {
        PBXFileReference *fileReference = new PBXFileReference(this);
        fileReference->setPath(filePath);
        fileReference->setName(filePath.mid(filePath.lastIndexOf(QLatin1Char('/')) + 1));
        d->children.append(fileReference);
        fileRefs.append(fileReference);
    }

    return fileRefs;
}

QString PBXGroup::comment() const
{
    return !path().isEmpty() ? path() : PBXReference::comment();
}

QByteArray PBXGroup::hashData() const
{
    QByteArray data = PBXReference::hashData();
    PBXGroup *parentGroup = dynamic_cast<PBXGroup *>(parent());
    if (parentGroup)
        data.prepend(parentGroup->hashData() + QByteArrayLiteral(":"));
    return data;
}

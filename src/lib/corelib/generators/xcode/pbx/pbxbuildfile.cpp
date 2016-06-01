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

#include "pbxbuildfile.h"
#include "pbxbuildphase.h"
#include "pbxfilereference.h"

static const QString kPBXBuildFileAttributes = QStringLiteral("ATTRIBUTES");
static const QString kPBXBuildFileLibraryWeakLinked = QStringLiteral("Weak");

class PBXBuildFilePrivate
{
public:
    PBXFileReference *reference = nullptr;
    QSet<QString> attributes;
};

PBXBuildFile::PBXBuildFile(PBXBuildPhase *parent) :
    PBXObject(parent), d(new PBXBuildFilePrivate)
{
}

PBXBuildFile::~PBXBuildFile()
{
    delete d;
}

QString PBXBuildFile::isa() const
{
    return QStringLiteral("PBXBuildFile");
}

PBXObjectMap PBXBuildFile::toMap() const
{
    PBXObjectMap self = PBXObject::toMap();
    if (d->reference)
        self.insert(QStringLiteral("fileRef"), QVariant::fromValue(d->reference->identifier()));

    QVariantMap settingsMap;
    if (!d->attributes.isEmpty()) {
        QStringList attributes = d->attributes.toList();
        qSort(attributes);
        settingsMap.insert(kPBXBuildFileAttributes, attributes);
    }

    if (!settingsMap.isEmpty())
        self.insert(QStringLiteral("settings"), settingsMap);

    return self;
}

PBXFileReference *PBXBuildFile::fileReference() const
{
    return d->reference;
}

void PBXBuildFile::setFileReference(PBXFileReference *reference)
{
    d->reference = reference;
}

QSet<QString> PBXBuildFile::attributes() const
{
    return d->attributes;
}

bool PBXBuildFile::isWeakLinkedLibrary() const
{
    return d->attributes.contains(kPBXBuildFileLibraryWeakLinked);
}

void PBXBuildFile::setWeakLinkedLibrary(bool weakLinked)
{
    if (weakLinked)
        d->attributes.insert(kPBXBuildFileLibraryWeakLinked);
    else
        d->attributes.remove(kPBXBuildFileLibraryWeakLinked);
}

QString PBXBuildFile::comment() const
{
    if (d->reference) {
        PBXBuildPhase *phase = dynamic_cast<PBXBuildPhase *>(parent());
        Q_ASSERT(phase);
        return QString(QStringLiteral("%1 in %2")).arg(d->reference->name()).arg(phase->name());
    }

    return PBXObject::comment();
}

QByteArray PBXBuildFile::hashData() const
{
    QByteArray data = PBXObject::hashData();
    if (d->reference)
        data.append(d->reference->hashData());
    return data;
}

/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2015 Jake Petroules.
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Build Suite.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "pbxbuildfile.h"
#include "pbxbuildphase.h"
#include "pbxfilereference.h"

static const QString kPBXBuildFileAttributes = QLatin1String("ATTRIBUTES");
static const QString kPBXBuildFileLibraryWeakLinked = QLatin1String("Weak");

class PBXBuildFilePrivate
{
public:
    PBXBuildFilePrivate();
    PBXFileReference *reference;
    QSet<QString> attributes;
};

PBXBuildFilePrivate::PBXBuildFilePrivate()
    : reference()
{
}

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
    return QLatin1String("PBXBuildFile");
}

PBXObjectMap PBXBuildFile::toMap() const
{
    PBXObjectMap self = PBXObject::toMap();
    if (d->reference) {
        self.insert(QLatin1String("fileRef"), QVariant::fromValue(d->reference->identifier()));
    }

    QVariantMap settingsMap;
    if (!d->attributes.isEmpty()) {
        QStringList attributes = d->attributes.toList();
        qSort(attributes);
        settingsMap.insert(kPBXBuildFileAttributes, attributes);
    }

    if (!settingsMap.isEmpty()) {
        self.insert(QLatin1String("settings"), settingsMap);
    }

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
        return QString(QLatin1String("%1 in %2")).arg(d->reference->name()).arg(phase->name());
    }

    return PBXObject::comment();
}

QByteArray PBXBuildFile::hashData() const
{
    QByteArray data = PBXObject::hashData();
    if (d->reference) {
        data.append(d->reference->hashData());
    }

    return data;
}

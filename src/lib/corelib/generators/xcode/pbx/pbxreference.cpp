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

#include "pbxreference.h"

class PBXReferencePrivate
{
public:
    QString name;
    QString path;
    PBX::PBXSourceTree sourceTree = static_cast<PBX::PBXSourceTree>(0);
    QString sourceTreeVariable;
};

PBXReference::PBXReference(QObject *parent) :
    PBXObject(parent), d(new PBXReferencePrivate)
{
    setSourceTree(PBX::Absolute);
}

PBXReference::~PBXReference()
{
    delete d;
}

PBXObjectMap PBXReference::toMap() const
{
    PBXObjectMap self = PBXObject::toMap();

    if (!name().isEmpty() && name() != path())
        self.insert(QStringLiteral("name"), name());

    // Xcode crashes if path is empty, even though it should never be
    if (!d->path.isEmpty())
        self.insert(QStringLiteral("path"), d->path); // relative path including the file name

    self.insert(QStringLiteral("sourceTree"), sourceTreeVariable());
    return self;
}

/*!
 * \brief PBXReference::name
 * \return display name of file or group, usually the same as the name of the file or directory.
 */
QString PBXReference::name() const
{
    return d->name;
}

void PBXReference::setName(const QString &name)
{
    d->name = name;
}

QString PBXReference::path() const
{
    return d->path;
}

void PBXReference::setPath(const QString &filePath)
{
    d->path = filePath;
}

PBX::PBXSourceTree PBXReference::sourceTree() const
{
    return d->sourceTree;
}

void PBXReference::setSourceTree(PBX::PBXSourceTree sourceTree)
{
    d->sourceTree = sourceTree;
    d->sourceTreeVariable = PBX::sourceTreeString(sourceTree);
}

QString PBXReference::sourceTreeVariable() const
{
    return d->sourceTreeVariable;
}

void PBXReference::setSourceTree(const QString &variableName)
{
    d->sourceTree = PBX::sourceTreeIdentifier(variableName);
    d->sourceTreeVariable = variableName;
}

QString PBXReference::comment() const
{
    return name();
}

QByteArray PBXReference::hashData() const
{
    QByteArray data = PBXObject::hashData();
    if (!d->path.isEmpty()) {
        QStringList pathComponents;
        const PBXReference *ref = this;
        do {
            pathComponents.insert(0, ref->path());
        } while ((ref = dynamic_cast<PBXReference *>(ref->parent())));

        data.append(pathComponents.join(QLatin1Char('/')).toUtf8());
    } else if (!d->name.isEmpty()) {
        data.append(d->name.toUtf8());
    }
    return data;
}

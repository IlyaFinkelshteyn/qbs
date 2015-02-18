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

#include "pbxreference.h"

class PBXReferencePrivate
{
public:
    PBXReferencePrivate();
    QString name;
    QString path;
    PBX::PBXSourceTree sourceTree;
};

PBXReferencePrivate::PBXReferencePrivate()
    : name(), sourceTree()
{
}

PBXReference::PBXReference(QObject *parent) :
    PBXObject(parent), d(new PBXReferencePrivate)
{
}

PBXReference::~PBXReference()
{
    delete d;
}

PBXObjectMap PBXReference::toMap() const
{
    PBXObjectMap self = PBXObject::toMap();

    if (!name().isEmpty() && name() != path()) {
        self.insert(QLatin1String("name"), name());
    }

    // Xcode crashes if path is empty, even though it should never be
    if (!d->path.isEmpty())
        self.insert(QLatin1String("path"), d->path); // relative path including the file name

    self.insert(QLatin1String("sourceTree"), PBX::sourceTreeString(sourceTree()));
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

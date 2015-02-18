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

#include "pbxobject.h"
#include <QCryptographicHash>
#include <QSet>
#include <QString>
#include <QUuid>

static QMap<QString, QByteArray> generatedIdentifiers;

class PBXObjectPrivate
{
public:
    QString identifier;
};

PBXObject::PBXObject(QObject *parent)
    : QObject(parent), d(new PBXObjectPrivate)
{
}

PBXObject::~PBXObject()
{
    delete d;
}

QString PBXObject::createIdentifier(const QByteArray &hashData)
{
    if (qgetenv("QBS_XCODE_GENERATOR_DETERMINISTIC_IDENTIFIERS") == QByteArray("1")) {
        QCryptographicHash hash(QCryptographicHash::Sha3_512);
        hash.addData(hashData);
        const QString result = QString::fromLatin1(hash.result().left(12).toHex().toUpper());
        if (generatedIdentifiers.contains(result)
                /*&& generatedIdentifiers.value(result) != hashData*/) {
            // hash collision!
            fprintf(stderr, "hash collision %s - %s\n", result.toUtf8().data(), hashData.data());
            abort();
        }
        generatedIdentifiers.insert(result, hashData);
        return result;
    }

    return QString::fromLatin1(QUuid::createUuid().toByteArray().left(12).toHex().toUpper());
}

PBXObjectIdentifier PBXObject::identifier() const
{
    if (d->identifier.isEmpty())
        d->identifier = createIdentifier(isa().toUtf8() + QByteArray(":") + hashData());
    return PBXObjectIdentifier(d->identifier, isa(), comment());
}

PBXObjectMap PBXObject::toMap() const
{
    PBXObjectMap self;
    self.insert(QLatin1String("isa"), isa());
    return self;
}

QString PBXObject::comment() const
{
    return isa();
}

QByteArray PBXObject::hashData() const
{
    return QByteArray();
}

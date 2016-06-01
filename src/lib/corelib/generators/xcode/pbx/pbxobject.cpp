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
        if (generatedIdentifiers.contains(result)) {
            // hash collision!
            fprintf(stderr,
                    "Xcode project generator hash collision for identifier %s.\nInput data: '%s'.\n"
                    "File a bug report and set the "
                    "QBS_XCODE_GENERATOR_NO_DETERMINISTIC_IDENTIFIERS\nenvironment variable to "
                    "work around.\n", qPrintable(result), hashData.data());
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
    self.insert(QStringLiteral("isa"), isa());
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

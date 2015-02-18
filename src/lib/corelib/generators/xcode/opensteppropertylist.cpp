/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Copyright (C) 2014 Petroules Corporation.
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

#include "opensteppropertylist.h"
#include "pbxobject.h"
#include <QDebug>
#include <QStack>

const QChar OpenStepPropertyList::NewlineToken = QLatin1Char('\n');
const QChar OpenStepPropertyList::DictionaryBeginToken = QLatin1Char('{');
const QChar OpenStepPropertyList::DictionarySeparatorToken = QLatin1Char(';');
const QChar OpenStepPropertyList::DictionaryEndToken = QLatin1Char('}');
const QChar OpenStepPropertyList::ArrayBeginToken = QLatin1Char('(');
const QChar OpenStepPropertyList::ArraySeparatorToken = QLatin1Char(',');
const QChar OpenStepPropertyList::ArrayEndToken = QLatin1Char(')');
const QChar OpenStepPropertyList::QuotedStringBeginToken = QLatin1Char('"');
const QChar OpenStepPropertyList::QuotedStringEndToken = QLatin1Char('"');
const QChar OpenStepPropertyList::DataBeginToken = QLatin1Char('<');
const QChar OpenStepPropertyList::DataEndToken = QLatin1Char('>');
const QChar OpenStepPropertyList::IndentationToken = QLatin1Char('\t');

static bool isValidUnquotedStringCharacter(QChar x) {
    return x < 0x80 && (x.isLetterOrNumber()
                        || x == QLatin1Char('_')
                        || x == QLatin1Char('$')
                        || x == QLatin1Char('/')
                        || x == QLatin1Char(':')
                        || x == QLatin1Char('.'));
}

static QString formatString(const QString &string)
{
    bool isQuoted = string.isEmpty();
    if (!isQuoted)
    {
        for (int i = 0; i < string.size(); ++i)
        {
            if (!isValidUnquotedStringCharacter(string[i]))
            {
                isQuoted = true;
                break;
            }
        }
    }

    QString outString = string;
    outString.replace(QLatin1String("\\"), QLatin1String("\\\\"));
    outString.replace(QLatin1String("\""), QLatin1String("\\\""));

    if (isQuoted)
        outString.prepend(OpenStepPropertyList::QuotedStringBeginToken);
    if (isQuoted)
        outString.append(OpenStepPropertyList::QuotedStringEndToken);

    return outString;
}

QString OpenStepPropertyList::toString(const QVariantMap &propertyList)
{
    PBXObjectMap newMap;
    Q_FOREACH (const QString &key, propertyList.keys()) {
        newMap.insert(key, propertyList[key]);
    }
    return toString(newMap);
}

QString OpenStepPropertyList::toString(const PBXObjectMap &propertyList)
{
    return toString(propertyList, 0) + NewlineToken;
}

QString OpenStepPropertyList::toString(const QVariantMap &propertyList, int indentationLevel)
{
    PBXObjectMap newMap;
    Q_FOREACH (const QString &key, propertyList.keys()) {
        newMap.insert(key, propertyList[key]);
    }
    return toString(newMap, indentationLevel);
}

QString OpenStepPropertyList::toString(const PBXObjectMap &propertyList, int indentationLevel)
{
    QString string(indentationLevel, IndentationToken);
    string += DictionaryBeginToken;

    const bool isFlat =
            propertyList[QLatin1String("isa")] == QLatin1String("PBXBuildFile") ||
            propertyList[QLatin1String("isa")] == QLatin1String("PBXFileReference");
    if (!isFlat) {
        string += NewlineToken;
    }

    QList<PBXObjectIdentifier> keys = propertyList.keys();

    // Push the isa property to the top of the list as it's always written first
    const int isaIndex = keys.indexOf(QLatin1String("isa"));
    if (isaIndex >= 0) {
        keys.removeAt(isaIndex);
        keys.insert(0, QLatin1String("isa"));
    }

    QStack<QString> isas;

    foreach (const PBXObjectIdentifier &vkey, keys)
    {
        const QVariant &object = propertyList[vkey];

        if (!vkey.isa().isEmpty()) {
            if (!isas.empty() && vkey.isa() != isas.top()) {
                string += QString(QLatin1String("/* End %1 section */\n")).arg(isas.top());
                isas.pop();
            }

            if (isas.empty()) {
                isas.push(vkey.isa());
                string += QString(QLatin1String("\n/* Begin %1 section */\n")).arg(vkey.isa());
            }
        }

        string += QString(!isFlat ? (indentationLevel + 1) : 0, IndentationToken);
        string += toString(QVariant::fromValue(vkey), -1);
        string += QLatin1String(" = ");

        if (object.canConvert<PBXObjectMap>() || object.type() == QVariant::Map || object.type() == QVariant::List ||
                object.type() == QVariant::StringList || object.type() == QVariant::ByteArray)
            string += toString(object, !isFlat ? (indentationLevel + 1) : 0).trimmed();
        else
            string += toString(object, 0);

        string += DictionarySeparatorToken;
        string += !isFlat ? NewlineToken : QLatin1Char(' ');
    }

    if (!isas.empty()) {
        string += QString(QLatin1String("/* End %1 section */\n")).arg(isas.top());
        isas.pop();
    }

    if (!isFlat) {
        string += QString(indentationLevel, IndentationToken);
    }

    string += DictionaryEndToken;
    return string;
}

QString OpenStepPropertyList::toString(const QList<QVariant> &propertyList)
{
    return toString(propertyList, 0) + NewlineToken;
}

QString OpenStepPropertyList::toString(const QList<QVariant> &propertyList, int indentationLevel)
{
    QString string(indentationLevel, IndentationToken);
    string += ArrayBeginToken;
    string += NewlineToken;

    for (int i = 0; i < propertyList.size(); ++i)
    {
        const QVariant &object = propertyList[i];
        string += toString(object, indentationLevel + 1);
        string += ArraySeparatorToken;
        string += NewlineToken;
    }

    string += QString(indentationLevel, IndentationToken) + ArrayEndToken;
    return string;
}

QString OpenStepPropertyList::toString(const QVariant &object, int indentationLevel)
{
    if (object.canConvert<PBXObjectIdentifier>()) {
        PBXObjectIdentifier objid = object.value<PBXObjectIdentifier>();
        QString s = toString(objid.identifier(), indentationLevel);
        if (!objid.comment().isEmpty()) {
            s += QLatin1String(" /* ");
            s += objid.comment().replace(QLatin1String("*/"), QLatin1String("* /"));
            s += QLatin1String(" */");
        }
        return s;
    }

    if (object.canConvert<PBXObjectMap>()) {
        return toString(object.value<PBXObjectMap>(), indentationLevel);
    }

    switch (object.type())
    {
    case QVariant::Bool:
        return object.toBool() ? QLatin1String("YES") : QLatin1String("NO");
    case QVariant::ByteArray:
        return toString(object.toByteArray(), indentationLevel);
    case QVariant::Date:
    case QVariant::DateTime:
    case QVariant::Time:
        return toString(object.toDateTime(), indentationLevel);
    case QVariant::Double:
        return QString::number(object.toDouble());
    case QVariant::Int:
        return QString::number(object.toInt());
    case QVariant::List:
    case QVariant::StringList:
        return toString(object.toList(), indentationLevel);
    case QVariant::LongLong:
        return QString::number(object.toLongLong());
    case QVariant::Map:
        return toString(object.toMap(), indentationLevel);
    case QVariant::String:
        return toString(object.toString(), indentationLevel);
    case QVariant::UInt:
        return QString::number(object.toUInt());
    case QVariant::ULongLong:
        return QString::number(object.toULongLong());
    default:
        qDebug() << "Unsupported property list object type" << object;
        abort();
    }
}

QString OpenStepPropertyList::toString(const QByteArray &object, int indentationLevel)
{
    QString string(indentationLevel, IndentationToken);
    string += DataBeginToken + QString::fromLatin1(object.toHex()) + DataEndToken;
    return string;
}

QString OpenStepPropertyList::toString(const QDateTime &object, int indentationLevel)
{
    return toString(object.toUTC().toString(Qt::ISODate), indentationLevel);
}

QString OpenStepPropertyList::toString(const QString &object, int indentationLevel)
{
    QString string(indentationLevel, IndentationToken);
    string += formatString(object);
    return string;
}

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

#ifndef OPENSTEPPROPERTYLIST_H
#define OPENSTEPPROPERTYLIST_H

#include <QByteArray>
#include <QDateTime>
#include <QList>
#include <QString>
#include <QVariant>

class PBXObjectIdentifier;
typedef QMap<PBXObjectIdentifier, QVariant> PBXObjectMap;

class OpenStepPropertyList
{
    Q_DISABLE_COPY(OpenStepPropertyList)
public:
    static const QChar NewlineToken;
    static const QChar DictionaryBeginToken;
    static const QChar DictionarySeparatorToken;
    static const QChar DictionaryEndToken;
    static const QChar ArrayBeginToken;
    static const QChar ArraySeparatorToken;
    static const QChar ArrayEndToken;
    static const QChar QuotedStringBeginToken;
    static const QChar QuotedStringEndToken;
    static const QChar DataBeginToken;
    static const QChar DataEndToken;
    static const QChar IndentationToken;

    static QString toString(const QVariantMap &propertyList);
    static QString toString(const PBXObjectMap &propertyList);
    static QString toString(const QList<QVariant> &propertyList);

private:
    OpenStepPropertyList();

    static QString toString(const QVariantMap &propertyList, int indentationLevel);
    static QString toString(const PBXObjectMap &propertyList, int indentationLevel);
    static QString toString(const QList<QVariant> &propertyList, int indentationLevel);
    static QString toString(const QVariant &object, int indentationLevel);
    static QString toString(const QByteArray &object, int indentationLevel);
    static QString toString(const QDateTime &object, int indentationLevel);
    static QString toString(const QString &object, int indentationLevel);
};

#endif // OPENSTEPPROPERTYLIST_H

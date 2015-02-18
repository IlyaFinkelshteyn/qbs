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

#include "xcsettings.h"

#include <QFile>
#include <QTextStream>
#include <QXmlStreamWriter>

class XCSettingsPrivate {
public:
    XCSettingsPrivate()
        : autocreateSchemes()
    {
    }

    bool autocreateSchemes;
};

XCSettings::XCSettings(QObject *parent)
    : QObject(parent)
    , d(new XCSettingsPrivate)
{
}

XCSettings::~XCSettings()
{
    delete d;
}

bool XCSettings::autocreateSchemes() const
{
    return d->autocreateSchemes;
}

void XCSettings::setAutocreateSchemes(bool autocreateSchemes)
{
    d->autocreateSchemes = autocreateSchemes;
}

bool XCSettings::serialize(const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        QXmlStreamWriter writer(&file);
        writer.setAutoFormatting(true);
        writer.setAutoFormattingIndent(-1);

        writer.writeStartDocument();
        writer.writeDTD(QLatin1String("<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">"));

        writer.writeStartElement(QLatin1String("plist"));
        writer.writeAttribute(QLatin1String("version"), QLatin1String("1.0"));
        writer.writeStartElement(QLatin1String("dict"));

        writer.writeTextElement(QLatin1String("key"), QLatin1String("IDEWorkspaceSharedSettings_AutocreateContextsIfNeeded"));
        writer.writeEmptyElement(d->autocreateSchemes ? QLatin1String("true") : QLatin1String("false"));

        writer.writeEndElement(); // </dict>
        writer.writeEndElement(); // </plist>
        writer.writeEndDocument();

        return file.error() == QFile::NoError;
    }

    return false;
}


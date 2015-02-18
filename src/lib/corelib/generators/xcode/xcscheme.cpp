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
#include "pbxproject.h"
#include "pbxtarget.h"
#include "xcscheme.h"
#include <QFile>
#include <QDir>
#include <QXmlStreamWriter>

static const QString yes = QLatin1String("YES");
static const QString no = QLatin1String("NO");

class XCSchemePrivate
{
public:
    XCSchemePrivate();
    PBXProject *project;
    PBXTarget *target;
    bool parallelizeBuildables;
    bool buildImplicitDependencies;
    QString customExecutableFilePath;
};

XCSchemePrivate::XCSchemePrivate()
    : project()
    , target()
    , parallelizeBuildables(true)
    , buildImplicitDependencies(true)
{
}

XCScheme::XCScheme(QObject *parent) :
    QObject(parent), d(new XCSchemePrivate)
{
}

XCScheme::~XCScheme()
{
    delete d;
}

PBXProject *XCScheme::project() const
{
    return d->project;
}

void XCScheme::setProject(PBXProject *project)
{
    d->project = project;
}

PBXTarget *XCScheme::target() const
{
    return d->target;
}

void XCScheme::setTarget(PBXTarget *target)
{
    d->target = target;
}

bool XCScheme::parallelizeBuildables() const
{
    return d->parallelizeBuildables;
}

void XCScheme::setParallelizeBuildables(bool parallelize)
{
    d->parallelizeBuildables = parallelize;
}

bool XCScheme::buildImplicitDependencies() const
{
    return d->buildImplicitDependencies;
}

void XCScheme::setBuildImplicitDependencies(bool build)
{
    d->buildImplicitDependencies = build;
}

QString XCScheme::customExecutableFilePath() const
{
    return d->customExecutableFilePath;
}

void XCScheme::setCustomExecutableFilePath(const QString &customExecutableFilePath)
{
    d->customExecutableFilePath = customExecutableFilePath;
}

bool XCScheme::serialize(const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        QXmlStreamWriter writer(&file);
        writer.setAutoFormatting(true);
        writer.setAutoFormattingIndent(2);

        writer.writeStartDocument();
        writer.writeStartElement(QLatin1String("Scheme"));

        // now the "actions"
        writer.writeStartElement(QLatin1String("BuildAction"));
        writer.writeAttribute(QLatin1String("parallelizeBuildables"), d->parallelizeBuildables ? yes : no);
        writer.writeAttribute(QLatin1String("buildImplicitDependencies"), d->buildImplicitDependencies ? yes : no);
        writer.writeStartElement(QLatin1String("BuildActionEntries"));
        writer.writeStartElement(QLatin1String("BuildActionEntry"));
        writer.writeAttribute(QLatin1String("buildForTesting"), yes);
        writer.writeAttribute(QLatin1String("buildForRunning"), yes);
        writer.writeAttribute(QLatin1String("buildForProfiling"), yes);
        writer.writeAttribute(QLatin1String("buildForArchiving"), yes);
        writer.writeAttribute(QLatin1String("buildForAnalyzing"), yes);
        writer.writeStartElement(QLatin1String("BuildableReference"));
        writer.writeAttribute(QLatin1String("BuildableIdentifier"), QLatin1String("primary"));

        QByteArray bytes;
        bytes.append(QLatin1String("BlueprintIdentifier:"));
        bytes.append(filePath);

        writer.writeAttribute(QLatin1String("BlueprintIdentifier"), PBXObject::createIdentifier(bytes));
        if (d->target) {
            writer.writeAttribute(QLatin1String("BuildableName"), d->target->name());
            writer.writeAttribute(QLatin1String("BlueprintName"), d->target->name());
        }
        if (d->project) {
            const QString relativePath = QDir(d->project->projectDirPath()).relativeFilePath(d->project->projectWrapperPath());
            writer.writeAttribute(QLatin1String("ReferencedContainer"), QString(QLatin1String("container:%1")).arg(relativePath));
        }
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();

        if (!d->customExecutableFilePath.isEmpty()) {
            writer.writeStartElement(QLatin1String("LaunchAction"));
            writer.writeStartElement(QLatin1String("PathRunnable"));
            writer.writeAttribute(QLatin1String("FilePath"), d->customExecutableFilePath);
            writer.writeEndElement();
            writer.writeEndElement();
        }

        writer.writeEndElement();
        writer.writeEndDocument();

        return file.error() == QFile::NoError;
    }

    return false;
}

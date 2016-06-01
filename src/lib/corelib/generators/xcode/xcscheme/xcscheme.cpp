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

#include "xcscheme.h"

#include "../pbx/pbxobject.h"
#include "../pbx/pbxproject.h"
#include "../pbx/pbxtarget.h"

#include <QFile>
#include <QDir>
#include <QProcessEnvironment>
#include <QXmlStreamWriter>

static const QString yes = QStringLiteral("YES");
static const QString no = QStringLiteral("NO");

class XCSchemePrivate
{
public:
    PBXProject *project = nullptr;
    PBXTarget *target = nullptr;
    bool parallelizeBuildables = true;
    bool buildImplicitDependencies = true;
    QString customExecutableFilePath;
    QProcessEnvironment runEnvironment;
};

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

QProcessEnvironment XCScheme::runEnvironment() const
{
    return d->runEnvironment;
}

void XCScheme::setRunEnvironment(const QProcessEnvironment &env)
{
    d->runEnvironment = env;
}

bool XCScheme::serialize(const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        QXmlStreamWriter writer(&file);
        writer.setAutoFormatting(true);
        writer.setAutoFormattingIndent(2);

        writer.writeStartDocument();
        writer.writeStartElement(QStringLiteral("Scheme"));

        // now the "actions"
        writer.writeStartElement(QStringLiteral("BuildAction"));
        writer.writeAttribute(QStringLiteral("parallelizeBuildables"), d->parallelizeBuildables ? yes : no);
        writer.writeAttribute(QStringLiteral("buildImplicitDependencies"), d->buildImplicitDependencies ? yes : no);
        writer.writeStartElement(QStringLiteral("BuildActionEntries"));
        writer.writeStartElement(QStringLiteral("BuildActionEntry"));
        writer.writeAttribute(QStringLiteral("buildForTesting"), yes);
        writer.writeAttribute(QStringLiteral("buildForRunning"), yes);
        writer.writeAttribute(QStringLiteral("buildForProfiling"), yes);
        writer.writeAttribute(QStringLiteral("buildForArchiving"), yes);
        writer.writeAttribute(QStringLiteral("buildForAnalyzing"), yes);
        writer.writeStartElement(QStringLiteral("BuildableReference"));
        writer.writeAttribute(QStringLiteral("BuildableIdentifier"), QStringLiteral("primary"));

        QByteArray bytes;
        bytes.append(QStringLiteral("BlueprintIdentifier:"));
        bytes.append(filePath);

        writer.writeAttribute(QStringLiteral("BlueprintIdentifier"), PBXObject::createIdentifier(bytes));
        if (d->target) {
            writer.writeAttribute(QStringLiteral("BuildableName"), d->target->name()); // TODO target exe or bundle name
            writer.writeAttribute(QStringLiteral("BlueprintName"), d->target->name());
        }
        if (d->project) {
            const QString relativePath = QDir(d->project->projectDirPath()).relativeFilePath(d->project->projectWrapperPath());
            writer.writeAttribute(QStringLiteral("ReferencedContainer"), QString(QStringLiteral("container:%1")).arg(relativePath));
        }
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();

        if (true /*isRunnable*/) {
            writer.writeStartElement(QStringLiteral("LaunchAction"));

            if (d->target) {
                writer.writeStartElement(QStringLiteral("BuildableProductRunnable"));
                writer.writeStartElement(QStringLiteral("BuildableReference"));
                if (d->target) {
                    writer.writeAttribute(QStringLiteral("BuildableName"), d->target->name());  // TODO target exe or bundle name
                    writer.writeAttribute(QStringLiteral("BlueprintName"), d->target->name());
                }
                if (d->project) {
                    const QString relativePath = QDir(d->project->projectDirPath()).relativeFilePath(d->project->projectWrapperPath());
                    writer.writeAttribute(QStringLiteral("ReferencedContainer"), QString(QStringLiteral("container:%1")).arg(relativePath));
                }
                writer.writeEndElement();
                writer.writeEndElement();
            } else if (!d->customExecutableFilePath.isEmpty()) {
                writer.writeStartElement(QStringLiteral("PathRunnable"));
                writer.writeAttribute(QStringLiteral("FilePath"), d->customExecutableFilePath);
                writer.writeEndElement();
            }

            writer.writeStartElement(QStringLiteral("EnvironmentVariables"));
            for (const QString &key : d->runEnvironment.keys()) {
                // Skip system environment
                if (QProcessEnvironment::systemEnvironment().value(key) == d->runEnvironment.value(key))
                    continue;

                writer.writeStartElement(QStringLiteral("EnvironmentVariable"));
                writer.writeAttribute(QStringLiteral("key"), key);
                writer.writeAttribute(QStringLiteral("value"), d->runEnvironment.value(key));
                writer.writeAttribute(QStringLiteral("isEnabled"), yes);
                writer.writeEndElement();
            }
            writer.writeEndElement();

            writer.writeEndElement();
        }

        writer.writeEndElement();
        writer.writeEndDocument();

        return file.error() == QFile::NoError;
    }

    return false;
}

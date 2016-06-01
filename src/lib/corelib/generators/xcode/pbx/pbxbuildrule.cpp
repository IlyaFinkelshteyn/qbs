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

#include "pbxbuildrule.h"
#include "pbxnativetarget.h"
#include <tools/hostosinfo.h>
#include <tools/shellutils.h>

class PBXBuildRulePrivate
{
public:
    QString compilerSpec = QStringLiteral("com.apple.compilers.proxy.script");
    QString name;
    QString filePatterns;
    QString fileType = QStringLiteral("pattern.proxy");
    QMap<QString, QStringList> outputFiles;
    QString script;
    bool editable = true;
};

PBXBuildRule::PBXBuildRule(PBXNativeTarget *parent)
    : PBXObject(parent), d(new PBXBuildRulePrivate)
{
}

PBXBuildRule::~PBXBuildRule()
{
    delete d;
}

QString PBXBuildRule::isa() const
{
    return QStringLiteral("PBXBuildRule");
}

PBXObjectMap PBXBuildRule::toMap() const
{
    PBXObjectMap self = PBXObject::toMap();
    self.insert(QStringLiteral("compilerSpec"), d->compilerSpec);
    if (!d->filePatterns.isEmpty())
        self.insert(QStringLiteral("filePatterns"), d->filePatterns);
    self.insert(QStringLiteral("fileType"), d->fileType);
    self.insert(QStringLiteral("isEditable"), d->editable);
    if (!d->name.isEmpty())
        self.insert(QStringLiteral("name"), d->name);
    self.insert(QStringLiteral("script"), d->script);

    const QStringList outputFilePaths = d->outputFiles.keys();
    self.insert(QStringLiteral("outputFiles"), outputFilePaths);

    QStringList outputFilesCompilerFlags;
    for (const QString &outputFilePath : outputFilePaths) {
        outputFilesCompilerFlags.append(qbs::Internal::shellQuote(
                                            compilerFlags(outputFilePath),
                                            qbs::Internal::HostOsInfo::HostOsMacos));
    }

    return self;
}

QByteArray PBXBuildRule::hashData() const
{
    QByteArray data = PBXObject::hashData();
    data.append(d->name);
    for (const QString &key : d->outputFiles.keys())
        data.append(key.toUtf8());
    return data;
}

QString PBXBuildRule::name() const
{
    return d->name;
}

void PBXBuildRule::setName(const QString &name)
{
    d->name = name;
}

bool PBXBuildRule::isEditable() const
{
    return d->editable;
}

void PBXBuildRule::setEditable(bool editable)
{
    d->editable = editable;
}

QString PBXBuildRule::compilerSpec() const
{
    return d->compilerSpec;
}

void PBXBuildRule::setCompilerSpec(const QString &compilerSpec)
{
    d->compilerSpec = compilerSpec;
}

QString PBXBuildRule::script() const
{
    return d->script;
}

void PBXBuildRule::setScript(const QString &sourceCode)
{
    d->script = sourceCode;
}

void PBXBuildRule::addOutputFile(const QString &filePath, const QStringList &compilerFlags)
{
    d->outputFiles.insert(filePath, compilerFlags);
}

QStringList PBXBuildRule::compilerFlags(const QString &outputFilePath) const
{
    return d->outputFiles.value(outputFilePath);
}

bool PBXBuildRule::containsOutputFile(const QString &filePath)
{
    return d->outputFiles.contains(filePath);
}

void PBXBuildRule::removeOutputFile(const QString &filePath)
{
    d->outputFiles.remove(filePath);
}

QString PBXBuildRule::filePatterns() const
{
    return d->filePatterns;
}

void PBXBuildRule::setFilePatterns(const QString &filePatterns)
{
    d->filePatterns = filePatterns;
}

QString PBXBuildRule::fileType() const
{
    return d->fileType;
}

void PBXBuildRule::setFileType(const QString &fileType)
{
    d->fileType = fileType;
}

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

#ifndef PBXBUILDRULE_H
#define PBXBUILDRULE_H

#include "pbxobject.h"
#include <QObject>
#include <QStringList>

class PBXNativeTarget;

class PBXBuildRulePrivate;

class PBXBuildRule : public PBXObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PBXBuildRule)
public:
    explicit PBXBuildRule(PBXNativeTarget *parent = 0);
    virtual ~PBXBuildRule();

    QString isa() const override;

    PBXObjectMap toMap() const override;

    QByteArray hashData() const override;

    QString name() const;
    void setName(const QString &name);

    bool isEditable() const;
    void setEditable(bool editable);

    QString filePatterns() const;
    void setFilePatterns(const QString &filePatterns);

    QString fileType() const;
    void setFileType(const QString &fileType);

    QString compilerSpec() const;
    void setCompilerSpec(const QString &compilerSpec);

    QString script() const;
    void setScript(const QString &sourceCode);

    void addOutputFile(const QString &filePath, const QStringList &compilerFlags = QStringList());
    QStringList compilerFlags(const QString &outputFilePath) const;
    bool containsOutputFile(const QString &filePath);
    void removeOutputFile(const QString &filePath);

private:
    PBXBuildRulePrivate *d;
};

#endif // PBXBUILDRULE_H

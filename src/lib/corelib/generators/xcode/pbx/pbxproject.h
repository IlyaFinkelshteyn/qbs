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

#ifndef PBXPROJECT_H
#define PBXPROJECT_H

#include "pbxcontainer.h"

class PBXTarget;
class XCConfigurationList;

class PBXProjectPrivate;

class PBXProject : public PBXContainer
{
    Q_OBJECT
    Q_DISABLE_COPY(PBXProject)
public:
    typedef struct {
        int major;
        int minor;
    } Version;

    explicit PBXProject(const QString &projectAbsolutePath, QObject *parent = 0);
    ~PBXProject();

    QString isa() const;
    PBXObjectMap toMap() const;

    QString comment() const;

    QByteArray hashData() const;

    PBXGroup *frameworksRefGroup() const;

    /*!
     * Group in the root of the Xcode file tree which contains references to product target files.
     *
     * For example, this will contain references to output application and library executables.
     */
    PBXGroup *productRefGroup() const;

    static bool isProjectWrapperExtension(const QString &extension);
    static PBXProject *projectWithFile(const QString &projectAbsolutePath);

    QList<PBXTarget *> targets() const;
    PBXTarget *targetNamed(const QString &targetName) const;

    void addTarget(PBXTarget *target);

    QString name() const;
    void setName(const QString &name);

    QVariantMap attributes() const;
    void setAttributes(const QVariantMap &attributes);

    QVariant attribute(const QString &name) const;
    void setAttribute(const QString &name, const QVariant &value);
    void removeAttribute(const QString &name);

    Version lastUpgradeCheck() const;
    void setLastUpgradeCheck(const Version &version);

    QString classPrefix() const;
    void setClassPrefix(const QString &classPrefix);

    QString organizationName() const;
    void setOrganizationName(const QString &organizationName);

    /*!
     * Returns the path of the .xcodeproj wrapper directory.
     */
    QString projectWrapperPath() const;

    /*!
     * Returns the path of the .pbxproj file.
     */
    QString projectFilePath() const;

    QString projectDirPath() const;
    void setProjectDirPath(const QString &projectDirPath);

    void setBuildSetting(const QString &key, const QVariant &value);

    XCConfigurationList *buildConfigurationList() const;

private:
    PBXProjectPrivate *d;
};

#endif // PBXPROJECT_H

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

    QString projectDirPath() const;
    void setProjectDirPath(const QString &projectDirPath);

    void setBuildSetting(const QString &key, const QVariant &value);

    XCConfigurationList *buildConfigurationList() const;
    bool writeToFileSystemProjectFile(bool projectWrite, bool userWrite, bool checkNeedsRevert);

private:
    PBXProjectPrivate *d;
};

#endif // PBXPROJECT_H

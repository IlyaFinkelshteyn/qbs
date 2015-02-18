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

#ifndef XCODECONFIGURATIONSETTING_H
#define XCODECONFIGURATIONSETTING_H

#include <QList>
#include <QString>
#include <QVariant>

#include <api/project.h>
#include <api/projectdata.h>

class PBXProject;
class PBXTarget;
class XCBuildConfiguration;
class XCConfigurationList;

namespace qbs {

/*!
 * \brief Represents the mapping of an Xcode build setting name to a qbs module and property name.
 */
struct XcodeBuildSettingMapping {
    typedef QVariant (*XcodeBuildSettingValueFunction)(const qbs::Project &project,
                                                       const qbs::ProductData &productData,
                                                       const qbs::GroupData &groupData);

    QString xcodeName;
    QString qbsModule;
    QStringList qbsNames;
    bool isList;
    bool serializedAsList;
    XcodeBuildSettingValueFunction function; // overrides previous 3

    XcodeBuildSettingMapping(const QString &xcodeName, const QString &qbsModule,
                 const QString &qbsName, bool serializedAsList = false)
        : xcodeName(xcodeName), qbsModule(qbsModule), qbsNames(qbsName), isList(false),
          serializedAsList(serializedAsList), function(NULL) {}

    XcodeBuildSettingMapping(const QString &xcodeName, const QString &qbsModule,
                 const QStringList &qbsNames, bool serializedAsList = true)
        : xcodeName(xcodeName), qbsModule(qbsModule), qbsNames(qbsNames), isList(true),
          serializedAsList(serializedAsList), function(NULL) {}

    XcodeBuildSettingMapping(const QString &xcodeName, XcodeBuildSettingValueFunction function,
                             bool serializedAsList)
        : xcodeName(xcodeName), isList(serializedAsList),
          serializedAsList(serializedAsList), function(function) {}

    void applyToProject(PBXProject *xcodeProject, const Project &project) const;
    void applyToTarget(PBXTarget *xcodeTarget,
                       const Project &project,
                       const ProductData &productData, const GroupData &groupData) const;
    void applyToConfiguration(XCConfigurationList *xcodeConfigurationList,
                              const Project &project,
                              const ProductData &productData = ProductData(),
                              const GroupData &groupData = GroupData()) const;
    void applyToConfiguration(XCBuildConfiguration *xcodeConfiguration,
                              const Project &project,
                              const ProductData &productData = ProductData(),
                              const GroupData &groupData = GroupData()) const;

    static QList<XcodeBuildSettingMapping> defaultBuildSettingsMap();
};

} // namespace qbs

#endif // XCODECONFIGURATIONSETTING_H

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

#ifndef XCODECONFIGURATIONSETTING_H
#define XCODECONFIGURATIONSETTING_H

#include <QList>
#include <QString>
#include <QVariant>

#include <api/project.h>
#include <api/projectdata.h>
#include <generators/generatordata.h>

class PBXProject;
class PBXTarget;
class XCBuildConfiguration;
class XCConfigurationList;

namespace qbs {

/*!
 * \brief Represents the mapping of an Xcode build setting name to a qbs module and property name.
 */
struct XcodeBuildSettingMapping {
    typedef QVariant (*XcodeBuildSettingValueFunction)(const GeneratableProject &p,
                                                       const qbs::Project &project,
                                                       const GeneratableProductData &gproduct,
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
          serializedAsList(serializedAsList), function(nullptr) {}

    XcodeBuildSettingMapping(const QString &xcodeName, const QString &qbsModule,
                 const QStringList &qbsNames, bool serializedAsList = true)
        : xcodeName(xcodeName), qbsModule(qbsModule), qbsNames(qbsNames), isList(true),
          serializedAsList(serializedAsList), function(nullptr) {}

    XcodeBuildSettingMapping(const QString &xcodeName, XcodeBuildSettingValueFunction function,
                             bool serializedAsList)
        : xcodeName(xcodeName), isList(serializedAsList),
          serializedAsList(serializedAsList), function(function) {}

    void applyToProject(PBXProject *xcodeProject,
                        const GeneratableProject &p,
                        const Project &project) const;
    void applyToTarget(PBXTarget *xcodeTarget,
                       const GeneratableProject &p,
                       const Project &project,
                       const GeneratableProductData &product,
                       const ProductData &productData, const GroupData &groupData) const;
    void applyToConfiguration(XCConfigurationList *xcodeConfigurationList,
                              const GeneratableProject &p,
                              const Project &project,
                              const GeneratableProductData &product = GeneratableProductData(),
                              const ProductData &productData = ProductData(),
                              const GroupData &groupData = GroupData()) const;
    void applyToConfiguration(XCBuildConfiguration *xcodeConfiguration,
                              const GeneratableProject &p,
                              const Project &project,
                              const GeneratableProductData &product = GeneratableProductData(),
                              const ProductData &productData = ProductData(),
                              const GroupData &groupData = GroupData()) const;

    static QList<XcodeBuildSettingMapping> defaultBuildSettingsMap();
};

} // namespace qbs

#endif // XCODECONFIGURATIONSETTING_H

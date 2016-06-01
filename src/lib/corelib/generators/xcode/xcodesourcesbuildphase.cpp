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

#include "xcodebuildsettingmapping.h"
#include "xcodegeneratorutils.h"
#include "xcodesourcesbuildphase.h"
#include "pbx/pbxbuildphase.h"
#include "pbx/pbxbuildrule.h"
#include "pbx/pbxfilereference.h"
#include "pbx/pbxfiletype.h"
#include "pbx/pbxgroup.h"
#include "pbx/pbxnativetarget.h"
#include "pbx/pbxproject.h"
#include "pbx/pbxtarget.h"
#include "pbx/xcconfigurationlist.h"

namespace qbs {

class XcodeSourcesBuildPhasePrivate {
public:
    PBXProject *xcodeProject;
    PBXTarget *xcodeTarget;
    bool isBuildableFileReference(PBXFileReference *ref);
    QList<PBXFileReference *> addGroupFiles(PBXGroup *xcodeGroup,
                                            const GroupData &groupData);
};

XcodeSourcesBuildPhase::XcodeSourcesBuildPhase(PBXProject *xcodeProject, PBXTarget *xcodeTarget)
    : d(QSharedPointer<XcodeSourcesBuildPhasePrivate>::create())
{
    d->xcodeProject = xcodeProject;
    d->xcodeTarget = xcodeTarget;
}

void XcodeSourcesBuildPhase::addFiles(const GeneratableProject &project,
                                      const GeneratableProductData &productData,
                                      PBXGroup *xcodeProductGroup)
{
    std::map<QString, QStringList> sourceFileEnabledConfigurations;

    // Set up all the groups
    for (auto it = std::begin(productData.data), end = std::end(productData.data); it != end; ++it) {
        for (const auto &groupData : it.value().groups()) {
            // Ignore internal support groups
            if (groupData.name().startsWith(QStringLiteral("io.qt.qbs.internal."))
                    || (groupData.name() == QStringLiteral("Provisioning Profiles")
                        && groupData.location().filePath().endsWith(QStringLiteral("xcode.qbs"))))
                continue;

            auto group = xcodeProductGroup;
            if (groupData.name() == productData.name()) {
                auto xcconfig = d->xcodeTarget->buildConfigurationList()->addBuildConfiguration(XcodeGeneratorUtils::xcodeConfigurationName(it.key()));

                // This group has the same name as the product itself so we'll just add the files
                // directly to the "product" group we created earlier instead of creating a new one
                for (const auto &setting : XcodeBuildSettingMapping::defaultBuildSettingsMap())
                    setting.applyToConfiguration(xcconfig,
                                                 project,
                                                 project.projects.value(it.key()),
                                                 productData,
                                                 productData.data.value(it.key()),
                                                 groupData);

                d->xcodeTarget->setProductName(d->xcodeTarget->buildSetting(QStringLiteral("PRODUCT_NAME"), it.key()).toString());
            } else {
                // Create a group for this group
                group = PBXGroup::groupWithName(groupData.name(), xcodeProductGroup);
            }

            // Add the source files to the group
            group->setSourceTree(PBX::Group);
            QList<PBXFileReference *> fileRefs;
            //if (!productFilesAlreadyAdded)
                fileRefs = d->addGroupFiles(group, groupData);

            // If the group is enabled, then we'll actually BUILD them in addition to merely listing them
            if (groupData.isEnabled()) {
                for (const auto &ref : fileRefs) {
                    if (d->isBuildableFileReference(ref))
                        d->xcodeTarget->defaultSourceCodeBuildPhase()->addReference(ref);

                    sourceFileEnabledConfigurations[ref->path()] << it.key();
                }
            }
        }
    }

    //xcconfig->setBuildSetting(QStringLiteral("EXCLUDED_SOURCE_FILE_NAMES"), flattenXcconfigList(QStringList(excludedSources.toList())));
}

QList<PBXFileReference *> XcodeSourcesBuildPhasePrivate::addGroupFiles(PBXGroup *xcodeGroup,
                                                                       const GroupData &groupData)
{
    QStringList relativeFilePaths;
    for (const auto &artifactData : groupData.allSourceArtifacts())
        relativeFilePaths += XcodeGeneratorUtils::makePathRelativeTo(artifactData.filePath(),
                                                                     xcodeProject->projectDirPath());

    // Add all the files in the group as file references so they show up in the project tree
    QList<PBXFileReference *> fileRefs = xcodeGroup->addFiles(relativeFilePaths);
    for (PBXFileReference *ref : fileRefs) {
        if (!ref->path().startsWith(QLatin1Char('/')))
            ref->setSourceTree(PBX::Group);
    }

    return fileRefs;
}

bool XcodeSourcesBuildPhasePrivate::isBuildableFileReference(PBXFileReference *ref) {
    const auto fileType = PBXFileType::typeForFileInfo(QFileInfo(ref->path()));

    // Header files might be buildable (for example if we're using Qt and have a moc rule)
    if (fileType == QStringLiteral("sourcecode.c.h")) {
        if (const auto nativeTarget = dynamic_cast<PBXNativeTarget *>(xcodeTarget)) {
            for (const auto &rule : nativeTarget->buildRules()) {
                if (rule->filePatterns() == QStringLiteral("*.h")) {
                    return true;
                }
            }
        }
    }

    return PBXFileType::isBuildableFileType(fileType);
}

} // namespace qbs

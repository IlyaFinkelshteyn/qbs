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
#include "xcodeframeworksbuildphase.h"
#include "xcodegenerator.h"
#include "xcodegeneratorutils.h"
#include "xcodeschemesvisitor.h"
#include "xcodesourcesbuildphase.h"

#include <qbs.h>
#include <api/runenvironment.h>
#include <generators/generatableprojectiterator.h>
#include <generators/igeneratableprojectvisitor.h>
#include <logging/logger.h>
#include <logging/translator.h>
#include <tools/filesaver.h>
#include <tools/qbsassert.h>
#include <tools/shellutils.h>
#include "pbx/pbx.h"
#include "io/pbxprojectwriter.h"
#include "xcsettings/xcsettings.h"
#include "xcscheme/xcscheme.h"

#include <QDebug>
#include <QDir>
#include <QTextStream>
#include <QXmlStreamWriter>

static const QString kQbsRunnerFileName = QStringLiteral("qbs");

namespace qbs {

using namespace Internal;

class XcodeGeneratorPrivate : public IGeneratableProjectVisitor {
public:
    XcodeGeneratorPrivate(XcodeGenerator *generator)
        : q(generator)
    {
    }

    void visitProject(const GeneratableProject &project) override;
    void visitProjectData(const GeneratableProject &project,
                          const GeneratableProjectData &parentProjectData,
                          const GeneratableProjectData &projectData) override;
    void visitProjectData(const ProjectData &projectData,
                          const QString &configuration) override;
    void visitProduct(const GeneratableProject &project,
                      const GeneratableProjectData &projectData,
                      const GeneratableProductData &productData) override;
    void visitProduct(const ProductData &productData,
                      const QString &configuration) override;

    void addTargetArtifacts(const GeneratableProject &project, const GeneratableProductData &product, PBXTarget *xcodeTarget);

    XcodeGenerator *q;
    QSharedPointer<PBXProject> xcodeProject;

    QMap<QString, PBXGroup *> m_projectGroups;
    QMap<QString, PBXGroup *> m_productGroups;
};

XcodeGenerator::XcodeGenerator()
    : d(QSharedPointer<XcodeGeneratorPrivate>::create(this))
{
}

static inline QVariant flattenXcconfigList(const QStringList &list) {
    if (list.isEmpty())
        return QString();
    if (list.size() == 1)
        return list.first();
    return list;
}

static void addQtRules(const GeneratableProductData &product, PBXNativeTarget *xcodeNativeTarget) {
    bool foundQt = false;
    for (auto it = std::begin(product.data), end = std::end(product.data); it != end; ++it) {
        const auto qtBinPath = it.value().moduleProperties().getModuleProperty(
                    QStringLiteral("Qt.core"), QStringLiteral("binPath")).toString();
        if (!qtBinPath.isEmpty()) {
            xcodeNativeTarget->setBuildSetting(QStringLiteral("QT_INSTALL_PREFIX"),
                                               QFileInfo(qtBinPath).path(), it.key());
            xcodeNativeTarget->setBuildSetting(QStringLiteral("QT_HOST_BINS"),
                                               qtBinPath, it.key());
            foundQt = true;
        }
    }

    if (foundQt) {
        PBXBuildRule *rule = new PBXBuildRule(xcodeNativeTarget);
        rule->setName(QStringLiteral("moc"));
        rule->setFilePatterns(QStringLiteral("*.h"));
        const QString outputFile = QStringLiteral("${DERIVED_FILE_DIR}/moc_${INPUT_FILE_BASE}.cpp");
        rule->addOutputFile(outputFile);
        rule->setScript(QStringLiteral("\"${QT_HOST_BINS}/moc\" -D Q_OS_MAC -I \"$SDKROOT/usr/include\" -nn -o \"") + outputFile + QStringLiteral("\" \"${INPUT_FILE_PATH}\""));
        xcodeNativeTarget->addBuildRule(rule);
    }
}

static inline ArtifactData runnableOrPrimaryTargetArtifactForProduct(const qbs::ProductData &qbsProductData) {
    // Easy in this case...
    if (qbsProductData.targetArtifacts().size() == 1)
        return qbsProductData.targetArtifacts().first();

    const QStringList types = QStringList()
            << QStringLiteral("bundle")
            << QStringLiteral("application")
            << QStringLiteral("dynamiclibrary")
            << QStringLiteral("staticlibrary")
            << QStringLiteral("loadablemodule");

    for (const auto &ta : qbsProductData.targetArtifacts()) {
        for (const auto &type : types) {
            if (qbsProductData.type().contains(type)
                    && ta.fileTags().contains(type))
                return ta;
        }
    }

    return ArtifactData();
}

void XcodeGenerator::generate()
{
    GeneratableProjectIterator it(project());
    it.accept(d.data());

    XcodeSchemesVisitor schemesVisitor(d->xcodeProject);
    it.accept(&schemesVisitor);

    const QString pbxprojFilePath = d->xcodeProject->projectFilePath();
    const QString xcodeprojWrapperName = QFileInfo(d->xcodeProject->projectWrapperPath()).fileName();

    Internal::FileSaver file(pbxprojFilePath);
    if (!file.open())
        throw ErrorInfo(Tr::tr("Cannot open %s for writing").arg(pbxprojFilePath));

    PBXProjectWriter writer(file.device());
    if (!(writer.write(d->xcodeProject.data()) && file.commit()))
        throw ErrorInfo(Tr::tr("Failed to generate %1").arg(pbxprojFilePath));

    qDebug() << "Generated" << qPrintable(xcodeprojWrapperName);
}

void XcodeGeneratorPrivate::visitProject(const GeneratableProject &project)
{
    const QString xcodeWrapperName = project.name() + QStringLiteral(".xcodeproj");
    QDir baseBuildDirectory = project.baseBuildDirectory();
    if (!baseBuildDirectory.cd(xcodeWrapperName))
        if (!baseBuildDirectory.mkdir(xcodeWrapperName)
                || !baseBuildDirectory.cd(xcodeWrapperName))
            throw ErrorInfo(Tr::tr("failed to make Xcode project wrapper directory"));

    const QString xcodeWorkspaceWrapperName = QStringLiteral("project.xcworkspace");
    const QString sharedWorkspaceDataDirName = QStringLiteral("xcshareddata");

    QDir sharedWorkspaceDataDir = baseBuildDirectory;
    if (!sharedWorkspaceDataDir.cd(xcodeWorkspaceWrapperName))
        if (!sharedWorkspaceDataDir.mkdir(xcodeWorkspaceWrapperName)
                || !sharedWorkspaceDataDir.cd(xcodeWorkspaceWrapperName))
            throw ErrorInfo(Tr::tr("failed to make Xcode workspace wrapper directory"));

    if (!sharedWorkspaceDataDir.cd(sharedWorkspaceDataDirName))
        if (!sharedWorkspaceDataDir.mkdir(sharedWorkspaceDataDirName)
                || !sharedWorkspaceDataDir.cd(sharedWorkspaceDataDirName))
            throw ErrorInfo(Tr::tr("failed to make Xcode workspace shared data directory"));

    const QString sharedWorkspaceSettingsFilePath = sharedWorkspaceDataDir.absoluteFilePath(
                QStringLiteral("WorkspaceSettings.xcsettings"));
    XCSettings sharedWorkspaceSettings;
    sharedWorkspaceSettings.setAutocreateSchemes(false);
    if (!sharedWorkspaceSettings.serialize(sharedWorkspaceSettingsFilePath))
        throw ErrorInfo(Tr::tr("Failed to generate workspace settings %1").arg(
                            sharedWorkspaceSettingsFilePath));

    // Create a shell script to invoke the qbs binary
    const QString &qbsRunnerFilePath = sharedWorkspaceDataDir.absoluteFilePath(kQbsRunnerFileName);
    QFile qbsRunner(qbsRunnerFilePath);
    if (qbsRunner.open(QIODevice::WriteOnly)) {
        // The runner script, in addition to invoking qbs itself,
        // adds symlinks from the Xcode target directory to the qbs build directory
        // This way, products in the Xcode files listing show up properly
        QTextStream ts(&qbsRunner);
        ts << QStringLiteral("#!/bin/bash\n")
           << QStringLiteral("set -e\n")
           << QStringLiteral("userscript=\"$PROJECT_FILE_PATH/project.xcworkspace/xcuserdata/$USER.xcuserdatad/qbs\"\n")
           << QStringLiteral("[ -f \"$userscript\" ] && . $userscript\n")
           << QStringLiteral("[ -x \"$QBS\" ] || exit 1\n")
           << QStringLiteral("\"$QBS\" \"$@\"\n")
           << QStringLiteral("if [ ! -e \"$TARGET_BUILD_DIR/$PRODUCT_NAME/$FULL_PRODUCT_NAME\" ] && [ -e \"$QBS_TARGET_PATH\" ] ; then\n")
           << QStringLiteral("mkdir -p \"$TARGET_BUILD_DIR/$PRODUCT_NAME\"\n")
           << QStringLiteral("ln -sf \"$QBS_TARGET_PATH\" \"$TARGET_BUILD_DIR/$PRODUCT_NAME/$FULL_PRODUCT_NAME\"\n")
           << QStringLiteral("fi\n");
    }

    xcodeProject = QSharedPointer<PBXProject>::create(baseBuildDirectory.absoluteFilePath(QStringLiteral("project.pbxproj")));
    xcodeProject->setName(project.name());

    // Only potentially add absolute paths if the qbs source directory and build directory differ;
    // this makes the Xcode project file relocatable (and thus version trackable) if generated in-source
    const QString qbsSourceDirectory = project.filePath().absolutePath();
    if (baseBuildDirectory.absolutePath() != qbsSourceDirectory) {
        // TODO: Make it relative anyways
        xcodeProject->setProjectDirPath(qbsSourceDirectory);
        xcodeProject->rootGroup()->setPath(xcodeProject->projectDirPath());
    }
}

void XcodeGeneratorPrivate::visitProjectData(const GeneratableProject &project,
                                             const GeneratableProjectData &parentProjectData,
                                             const GeneratableProjectData &projectData)
{
    PBXGroup *parentGroup = nullptr;
    if (parentProjectData.isValid())
        parentGroup = m_projectGroups.value(parentProjectData.name());
    if (!parentGroup)
        parentGroup = xcodeProject->rootGroup();

    auto baseGroup = m_projectGroups.value(projectData.name());
    if (!baseGroup) {
        baseGroup = PBXGroup::groupWithName(projectData.name(), parentGroup);
        baseGroup->setSourceTree(PBX::Group);
        m_projectGroups.insert(projectData.name(), baseGroup);
    }

    // Add the qbs project file for this project
    for (auto ref : baseGroup->addFiles(QStringList() << XcodeGeneratorUtils::makePathRelativeTo(project.filePath().absoluteFilePath(), xcodeProject->projectDirPath()))) {
        if (!ref->path().startsWith(QLatin1Char('/')))
            ref->setSourceTree(PBX::Group);
    }

    project.forEach([&project, this](const QString &configurationName, const Project &qproject) {
        XCBuildConfiguration *xcconfig = xcodeProject->buildConfigurationList()->addBuildConfiguration(configurationName);
        xcconfig->setBuildSetting(QStringLiteral("QBS_CONFIGURATION_PARAMETERS"), QStringList(project.commandLines[configurationName].mid(1)));
        for (const XcodeBuildSettingMapping &setting : XcodeBuildSettingMapping::defaultBuildSettingsMap())
            ;//setting.applyToConfiguration(xcconfig, project, qproject);
        // missing params default constructors bad!
    });

    projectData.forEach([&project, this](const QString &configurationName, const ProjectData &) {
        XCBuildConfiguration *xcconfig = xcodeProject->buildConfigurationList()->addBuildConfiguration(configurationName);
        xcconfig->setBuildSetting(QStringLiteral("QBS_PROJECT_FILE_NAME"), project.filePath().fileName());
    });
}

void XcodeGeneratorPrivate::visitProjectData(const ProjectData &projectData,
                                             const QString &configuration)
{


    XCBuildConfiguration *xcconfig = xcodeProject->buildConfigurationList()->addBuildConfiguration(configuration);
    xcodeProject->buildConfigurationList()->setDefaultBuildConfiguration(xcconfig->name());

    // TODO: Ideally should go into a configuration file instead of in the project file itself
    // TODO: Is there a better way to find the qbs binary to invoke? For example, someone could use libqbscore to invoke generators...
    xcconfig->setBuildSetting(QStringLiteral("QBS"), QCoreApplication::applicationFilePath());

    // TODO: Why we need this??!?! Nov 26 night
    //xcconfig->setBuildSetting(QStringLiteral("QBS_PROJECT_DIRECTORY"), xcodeInsertPlaceholdersInValue(project, qbsSourceDirectory));


    // or baseBuildDirectory + profileName + '-' + buildVariant
    xcconfig->setBuildSetting(QStringLiteral("__QBS_BASE_BUILD_DIRECTORY"), QStringLiteral("$(PROJECT_FILE_PATH)/.."));
    xcconfig->setBuildSetting(QStringLiteral("QBS_BUILD_DIRECTORY"), QStringLiteral("$(__QBS_BASE_BUILD_DIRECTORY:standardizepath)"));
}

void XcodeGeneratorPrivate::visitProduct(const GeneratableProject &project,
                                         const GeneratableProjectData &projectData,
                                         const GeneratableProductData &productData)
{
    PBXTarget *xcodeTarget = q->xcodeTargetForProduct(xcodeProject.data(), productData);
    xcodeTarget->setName(productData.name());
    xcodeTarget->setProductType(XcodeGeneratorUtils::xcodeProductType(productData));

    PBXLegacyTarget *xcodeLegacyTarget = dynamic_cast<PBXLegacyTarget *>(xcodeTarget);
    if (xcodeLegacyTarget) {
        xcodeLegacyTarget->setBuildWorkingDirectory(XcodeGeneratorUtils::xcodeInsertPlaceholdersInValue(project, xcodeProject->projectDirPath()).toString());
        xcodeLegacyTarget->setBuildToolPath(QStringLiteral("/bin/bash"));

        CommandLine commandLine;
        commandLine.setProgram(QStringLiteral("\"$(PROJECT_FILE_PATH)\"/project.xcworkspace/xcshareddata/") + kQbsRunnerFileName, true);
        commandLine.appendRawArgument(QStringLiteral("\"$(ACTION)\""));
        commandLine.appendArgument(QStringLiteral("-f"));

        // Can't use xcodeInsertPlaceholdersInValue because we want to do a little acrobatics to get quoting 100% right...
        const QDir sourceDirectory(xcodeProject->projectDirPath());
        QString resultingFilePath = project.filePath().absoluteFilePath();
        if (resultingFilePath.startsWith(sourceDirectory.absolutePath())) {
            resultingFilePath.replace(0, sourceDirectory.absolutePath().size(), QString());
            commandLine.appendRawArgument(QStringLiteral("\"$(SRCROOT)\"")
                                             + shellQuote(resultingFilePath,
                                                          HostOsInfo::HostOsMacos));
        } else {
            commandLine.appendPathArgument(resultingFilePath);
        }

        commandLine.appendArgument(QStringLiteral("-d"));
        commandLine.appendRawArgument(QStringLiteral("\"$(CONFIGURATION_BUILD_DIR)\""));
        commandLine.appendArgument(QStringLiteral("-p"));
        commandLine.appendRawArgument(QStringLiteral("\"$(TARGET_NAME)\"")); // target and product name are opposites in qbs and Xcode
        commandLine.appendRawArgument(QStringLiteral("\"$(CONFIGURATION)\""));
        commandLine.appendRawArgument(QStringLiteral("$(QBS_CONFIGURATION_PARAMETERS)"));

        xcodeLegacyTarget->setBuildArgumentsString(
                    commandLine.toCommandLine(HostOsInfo::HostOsMacos));
        xcodeLegacyTarget->setPassBuildSettingsInEnvironment(true);
    }

    PBXNativeTarget *xcodeNativeTarget = dynamic_cast<PBXNativeTarget *>(xcodeTarget);
    if (xcodeNativeTarget)
        addQtRules(productData, xcodeNativeTarget);

    // TODO: If a product with the same name was already added, don't add its files AGAIN
    // Create a root PBX group for the target's files and other groups
    PBXGroup *xcodeProductGroup = m_productGroups.value(productData.name());
    const bool productFilesAlreadyAdded = !!xcodeProductGroup;
    if (!productFilesAlreadyAdded) {
        // TODO wats this for?
        // TODO why was this marked "wats this for"? clearly it creates the group...
        m_productGroups.insert(productData.name(), xcodeProductGroup = PBXGroup::groupWithName(productData.name(), m_projectGroups.value(projectData.name())));

        // Add the qbs project file for this product
        for (PBXFileReference *ref : xcodeProductGroup->addFiles(QStringList() << XcodeGeneratorUtils::makePathRelativeTo(productData.location().filePath(), xcodeProject->projectDirPath()))) {
            if (!ref->path().startsWith(QLatin1Char('/')))
                ref->setSourceTree(PBX::Group);
        }
    }

    XcodeSourcesBuildPhase sourcesBuildPhase(xcodeProject.data(), xcodeTarget);
    sourcesBuildPhase.addFiles(project, productData, xcodeProductGroup);

    XcodeFrameworksBuildPhase frameworksBuildPhase(xcodeProject.data(), xcodeTarget);
    frameworksBuildPhase.addLinkables(productData);

    addTargetArtifacts(project, productData, xcodeTarget);

    xcodeProject->addTarget(xcodeTarget);
}

void XcodeGeneratorPrivate::visitProduct(const ProductData &productData,
                                         const QString &configuration)
{

}

void XcodeGeneratorPrivate::addTargetArtifacts(const GeneratableProject &project,
                                               const GeneratableProductData &product,
                                               PBXTarget *xcodeTarget)
{
    auto xcodeLegacyTarget = dynamic_cast<PBXLegacyTarget *>(xcodeTarget);

    product.forEach([&](const QString &configurationName, const qbs::ProductData &productData) {
        // Xcode can't handle any per-configuration differences here,
        // so we have to pick a configuration to apply
        if (!XcodeGeneratorUtils::isXcodeDefaultConfiguration(configurationName, product.data.keys()))
            return;

        // Add a reference for the main target artifact in root "Products" group
        auto artifact = runnableOrPrimaryTargetArtifactForProduct(productData);
        auto realArtifactPath = artifact.installData().installFilePath();
        if (!realArtifactPath.isEmpty()) {
            // "simple" generator
            if (!xcodeLegacyTarget) {
                // Chop off the Qbs build dir from the artifact path, because they will actually end
                // up in Xcode's build directory, not the build directory Qbs would use
                const QString buildDirectory = project.projectData(configurationName).buildDirectory();
                if (realArtifactPath.startsWith(buildDirectory))
                    realArtifactPath.remove(0, buildDirectory.size());

                if (realArtifactPath.startsWith(QLatin1Char('/')))
                    realArtifactPath.remove(0, 1);
            } else {
                realArtifactPath = XcodeGeneratorUtils::xcodeInsertPlaceholdersInValue(project, realArtifactPath).toString();
            }

            QBS_CHECK(!realArtifactPath.isEmpty());

            QList<PBXFileReference *> refs = xcodeProject->productRefGroup()->addFiles(QStringList() << realArtifactPath);
            for (PBXFileReference *ref : refs) {
                if (!xcodeLegacyTarget) {
                    ref->setSourceTree(PBX::BuildProductDir);

                    // This causes Xcode to force the file reference to a relative path relative to the build products dir
                    // Therefore, we must symlink artifacts in the Xcode build directory to the ones in the qbs build directory
                    // NOTE: This will crash Xcode if set for a PBXLegacyTarget (claiming an empty string global hex ID)
                    xcodeTarget->setProductReference(ref);
                }

                // TODO: Technically this should be composed of other variables based on the product type
                // $(WRAPPER_NAME) for bundles, $(EXECUTABLE_NAME) otherwise?
                XCBuildConfiguration *xcconfig = xcodeTarget->buildConfigurationList()->addBuildConfiguration(XcodeGeneratorUtils::xcodeConfigurationName(configurationName));
                xcconfig->setBuildSetting(QStringLiteral("FULL_PRODUCT_NAME"), QFileInfo(realArtifactPath).fileName());
                xcconfig->setBuildSetting(QStringLiteral("QBS_TARGET_PATH"), XcodeGeneratorUtils::xcodeInsertPlaceholdersInValue(project, realArtifactPath));
            }
        }
    });
}

PBXTarget *XcodeGenerator::xcodeTargetForProduct(PBXProject *xcodeProject,
                                                 const GeneratableProductData &product)
{
    Q_UNUSED(xcodeProject);
    PBXTarget *target = xcodeProject->targetNamed(product.name());

    // Native targets without a type won't build, so use a legacy target in this case
    if (!target && XcodeGeneratorUtils::xcodeProductType(product).isEmpty()) {
        return new PBXLegacyTarget(xcodeProject);
    }

    return target;
}

} // namespace qbs

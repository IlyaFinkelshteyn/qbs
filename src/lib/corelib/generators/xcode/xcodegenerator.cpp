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

#include "xcodebuildsettingmapping.h"
#include "xcodegenerator.h"

#include <qbs.h>
#include <logging/logger.h>
#include <logging/translator.h>
#include <tools/qbsassert.h>
#include <tools/shellutils.h>
#include "pbx.h"

#include <QDebug>
#include <QDir>
#include <QTextStream>
#include <QXmlStreamWriter>

static const QString kQbsRunnerFileName = QLatin1String("qbs");

namespace qbs {

using namespace Internal;

XcodeGenerator::XcodeGenerator()
{
}

static QString buildVariant(const qbs::Project &project)
{
    return project.projectConfiguration()[QLatin1String("qbs")].toMap()[QLatin1String("buildVariant")].toString();
}

static QString buildVariant(const qbs::ProductData &product)
{
    return product.moduleProperties().getModuleProperty(QLatin1String("qbs"), QLatin1String("buildVariant")).toString();
}

void XcodeGenerator::setupGenerator()
{
    QSet<QString> profileNames;
    QSet<QString> projectNames;
    QSet<QString> qbsProjectFiles;
    QSet<QString> buildDirectories;

    foreach (const qbs::Project &proj, projects()) {
        profileNames << proj.profile();
        projectNames << proj.projectData().name();
        qbsProjectFiles << proj.projectData().location().filePath();

        QDir baseBuildDirectory(proj.projectData().buildDirectory());
        baseBuildDirectory.cdUp();
        buildDirectories << baseBuildDirectory.absolutePath();
    }

    QBS_CHECK(!profileNames.isEmpty());
    QBS_CHECK(projectNames.size() == 1);
    QBS_CHECK(qbsProjectFiles.size() == 1);
    QBS_CHECK(buildDirectories.size() == 1);

    m_multipleProfiles = profileNames.size() > 1;
    m_projectName = projectNames.toList().first();
    m_qbsProjectFile = qbsProjectFiles.toList().first();
    m_baseBuildDirectory = buildDirectories.toList().first();

    QBS_CHECK(m_qbsProjectFile.isAbsolute() && m_qbsProjectFile.exists());
    QBS_CHECK(m_baseBuildDirectory.isAbsolute() && m_baseBuildDirectory.exists());

    m_productGroups.clear();
}

static inline TargetArtifact runnableOrPrimaryTargetArtifactForProduct(const qbs::ProductData &qbsProductData) {
    // Easy in this case...
    if (qbsProductData.targetArtifacts().size() == 1) {
        return qbsProductData.targetArtifacts().first();
    }

    const QStringList types = QStringList()
            << QLatin1String("bundle")
            << QLatin1String("application")
            << QLatin1String("dynamiclibrary")
            << QLatin1String("staticlibrary")
            << QLatin1String("loadablemodule");

    Q_FOREACH (const TargetArtifact &ta, qbsProductData.targetArtifacts()) {
        Q_FOREACH (const QString &type, types) {
            if (qbsProductData.type().contains(type)
                    && ta.fileTags().contains(type)) {
                return ta;
            }
        }
    }

    return TargetArtifact();
}

static inline QString installedFilePathForTargetArtifact(const qbs::Project &project,
                                                         const qbs::ProductData &product,
                                                         const qbs::InstallOptions &installOptions,
                                                         const TargetArtifact &ta) {
    const QList<InstallableFile> &installables
            = project.installableFilesForProduct(product, installOptions);
    foreach (const InstallableFile &file, installables) {
        if (file.sourceFilePath() == ta.filePath())
            return file.targetFilePath();
    }
    return ta.filePath();
}

void XcodeGenerator::generate(const InstallOptions &installOptions)
{
    // TODO: Move to base class and call ProjectGenerator::setupGenerator()
    setupGenerator();

    const QString xcodeWrapperName = m_projectName + QLatin1String(".xcodeproj");
    if (!m_baseBuildDirectory.cd(xcodeWrapperName))
        if (!m_baseBuildDirectory.mkdir(xcodeWrapperName) || !m_baseBuildDirectory.cd(xcodeWrapperName))
            throw ErrorInfo(Tr::tr("failed to make Xcode project wrapper directory"));

    const QString xcodeWorkspaceWrapperName = QLatin1String("project.xcworkspace");
    const QString sharedWorkspaceDataDirName = QLatin1String("xcshareddata");

    QDir sharedWorkspaceDataDir = m_baseBuildDirectory;
    if (!sharedWorkspaceDataDir.cd(xcodeWorkspaceWrapperName))
        if (!sharedWorkspaceDataDir.mkdir(xcodeWorkspaceWrapperName) || !sharedWorkspaceDataDir.cd(xcodeWorkspaceWrapperName))
            throw ErrorInfo(Tr::tr("failed to make Xcode workspace wrapper directory"));

    if (!sharedWorkspaceDataDir.cd(sharedWorkspaceDataDirName))
        if (!sharedWorkspaceDataDir.mkdir(sharedWorkspaceDataDirName) || !sharedWorkspaceDataDir.cd(sharedWorkspaceDataDirName))
            throw ErrorInfo(Tr::tr("failed to make Xcode workspace shared data directory"));

    const QString sharedWorkspaceSettingsFilePath = sharedWorkspaceDataDir.absoluteFilePath(QLatin1String("WorkspaceSettings.xcsettings"));
    XCSettings sharedWorkspaceSettings;
    sharedWorkspaceSettings.setAutocreateSchemes(false);
    if (!sharedWorkspaceSettings.serialize(sharedWorkspaceSettingsFilePath)) {
        throw ErrorInfo(Tr::tr("Failed to generate workspace settings %1").arg(sharedWorkspaceSettingsFilePath));
    }

    // Create a shell script to invoke the qbs binary
    // This is necessary because qbs hangs when invoked directly from Xcode
    // TODO: Why?
    const QString &qbsRunnerFilePath = m_baseBuildDirectory.absoluteFilePath(kQbsRunnerFileName);
    QFile qbsRunner(qbsRunnerFilePath);
    if (qbsRunner.open(QIODevice::WriteOnly)) {
        // The runner script, in addition to invoking qbs itself,
        // adds symlinks from the Xcode target directory to the qbs build directory
        // This way, products in the Xcode files listing show up properly
        QTextStream ts(&qbsRunner);
        ts << QLatin1String("#!/bin/bash\n")
           << QLatin1String("set -e\n")
           << QLatin1String("[ -x \"$QBS\" ] || exit 1\n")
           << QLatin1String("\"$QBS\" \"$@\"\n")
           << QLatin1String("if [ ! -e \"$TARGET_BUILD_DIR/$PRODUCT_NAME/$FULL_PRODUCT_NAME\" ] && [ -e \"$QBS_TARGET_PATH\" ] ; then\n")
           << QLatin1String("mkdir -p \"$TARGET_BUILD_DIR/$PRODUCT_NAME\"\n")
           << QLatin1String("ln -sf \"$QBS_TARGET_PATH\" \"$TARGET_BUILD_DIR/$PRODUCT_NAME/$FULL_PRODUCT_NAME\"\n")
           << QLatin1String("fi\n");
    }

    PBXProject xcodeProject(m_baseBuildDirectory.absoluteFilePath(QLatin1String("project.pbxproj")));
    xcodeProject.setName(m_projectName);

    // Only potentially add absolute paths if the qbs source directory and build directory differ;
    // this makes the Xcode project file relocatable (and thus version trackable) if generated in-source
    const QString qbsSourceDirectory = m_qbsProjectFile.absolutePath();
    if (m_baseBuildDirectory.absolutePath() != qbsSourceDirectory) {
        // TODO: Make it relative anyways
        xcodeProject.setProjectDirPath(qbsSourceDirectory);
        xcodeProject.rootGroup()->setPath(xcodeProject.projectDirPath());
    }

    foreach (const qbs::Project &project, projects()) {
        XCBuildConfiguration *xcconfig = xcodeProject.buildConfigurationList()->addBuildConfiguration(xcodeConfigurationName(project));
        xcodeProject.buildConfigurationList()->setDefaultBuildConfiguration(xcconfig->name());

        // TODO: Ideally should go into a configuration file instead of in the project file itself
        // TODO: Is there a better way to find the qbs binary to invoke? For example, someone could use libqbscore to invoke generators...
        xcconfig->setBuildSetting(QLatin1String("QBS"), QCoreApplication::applicationFilePath());
        xcconfig->setBuildSetting(QLatin1String("QBS_PROFILE"), project.profile());
        xcconfig->setBuildSetting(QLatin1String("QBS_PROJECT_DIRECTORY"), xcodeInsertPlaceholdersInValue(project, qbsSourceDirectory));
        xcconfig->setBuildSetting(QLatin1String("QBS_PROJECT_FILE_NAME"), m_qbsProjectFile.fileName());

        // or baseBuildDirectory + profileName + '-' + buildVariant
        xcconfig->setBuildSetting(QLatin1String("__QBS_BASE_BUILD_DIRECTORY"), QLatin1String("$(PROJECT_FILE_PATH)/.."));
        xcconfig->setBuildSetting(QLatin1String("QBS_BUILD_DIRECTORY"), QLatin1String("$(__QBS_BASE_BUILD_DIRECTORY:standardizepath)"));

        xcconfig->setBuildSetting(QLatin1String("QBS_BUILD_VARIANT"), buildVariant(project));

        foreach (const XcodeBuildSettingMapping &setting, XcodeBuildSettingMapping::defaultBuildSettingsMap()) {
            setting.applyToConfiguration(xcconfig, project);
        }

        // Add all the Qbs products as Xcode targets
        addProjectRecursive(project, project.projectData(), installOptions, xcodeProject, xcodeProject.rootGroup());

        // Do some per-target ops
        foreach (const qbs::ProductData &qbsProductData, project.projectData().allProducts()) {
            PBXTarget *qbsXcodeTarget = xcodeProject.targetNamed(qbsProductData.name());
            if (!qbsXcodeTarget)
                throw new qbs::ErrorInfo(Tr::tr("could not find PBXTarget for qbs product '%1'").arg(qbsProductData.name()));

            // Set up target dependencies
            foreach (const QString &dependencyName, qbsProductData.dependencies()) {
                PBXTargetDependency *dependentXcodeTarget = new PBXTargetDependency(&xcodeProject);
                dependentXcodeTarget->setTarget(xcodeProject.targetNamed(dependencyName));
                if (!dependentXcodeTarget->target())
                    throw new qbs::ErrorInfo(Tr::tr("could not find PBXTarget for qbs product '%1'").arg(dependencyName));

                qbsXcodeTarget->targetDependencies().append(dependentXcodeTarget);
            }

            // Set up a scheme for the target
            static const QString xcshareddata = QLatin1String("xcshareddata");
            static const QString xcschemes = QLatin1String("xcschemes");
            QDir schemesDirectory(m_baseBuildDirectory);
            if (!schemesDirectory.cd(xcshareddata)) {
                if (!schemesDirectory.mkdir(xcshareddata) || !schemesDirectory.cd(xcshareddata)) {
                    throw ErrorInfo(Tr::tr("failed to make Xcode project shared data directory"));
                }
            }
            if (!schemesDirectory.cd(xcschemes)) {
                if (!schemesDirectory.mkdir(xcschemes) || !schemesDirectory.cd(xcschemes)) {
                    throw ErrorInfo(Tr::tr("failed to make Xcode project shared schemes directory"));
                }
            }

            const QString schemeFilePath(schemesDirectory.absoluteFilePath(QString(QLatin1String("%1.xcscheme")).arg(qbsXcodeTarget->name())));
            XCScheme xcscheme;
            xcscheme.setParallelizeBuildables(false); // this must be turned off because it conflicts with qbs build directory locking
            xcscheme.setProject(&xcodeProject);
            xcscheme.setTarget(qbsXcodeTarget);

            if (dynamic_cast<PBXLegacyTarget *>(qbsXcodeTarget)) {
                xcscheme.setCustomExecutableFilePath(installedFilePathForTargetArtifact(project, qbsProductData, installOptions, runnableOrPrimaryTargetArtifactForProduct(qbsProductData)));
            }

            if (!xcscheme.serialize(schemeFilePath)) {
                throw ErrorInfo(Tr::tr("Failed to generate scheme %1").arg(schemeFilePath));
            }
        }
    }

    if (xcodeProject.writeToFileSystemProjectFile(true, true, true))
        qDebug() << "Generated" << qPrintable(xcodeWrapperName);
    else
        throw ErrorInfo(Tr::tr("Failed to generate %1").arg(xcodeWrapperName));
}

QString makePathRelativeTo(const QString &path, const QString &basePath)
{
    QString rootPath(basePath);
    if (!rootPath.endsWith(QLatin1Char('/')))
        rootPath += QLatin1Char('/');

    QString filePath(path);
    if (filePath.startsWith(rootPath))
        filePath.remove(0, rootPath.size());

    return filePath;
}

void XcodeGenerator::addProjectRecursive(const Project &project, const ProjectData &projectData, const InstallOptions &installOptions, PBXProject &xcodeProject, PBXGroup *xcodeGroup)
{
    foreach (const ProjectData &subProjectData, projectData.subProjects())
    {
        PBXGroup *baseGroup = PBXGroup::groupWithName(subProjectData.name(), xcodeGroup);
        baseGroup->setSourceTree(PBX::Group);

        // Add the qbs project file for this project
        foreach (PBXFileReference *ref, baseGroup->addFiles(QStringList() << makePathRelativeTo(m_qbsProjectFile.absoluteFilePath(), xcodeProject.projectDirPath()), false, false)) {
            if (!ref->path().startsWith(QLatin1Char('/'))) {
                ref->setSourceTree(PBX::Group);
            }
        }

        addProjectRecursive(project, subProjectData, installOptions, xcodeProject, baseGroup);
    }

    foreach (const ProductData &product, projectData.products())
    {
        addProduct(project, product, installOptions, xcodeProject, xcodeGroup);
    }
}

static inline QVariant flattenXcconfigList(const QStringList &list) {
    if (list.isEmpty())
        return QString();
    if (list.size() == 1)
        return list.first();
    return list;
}

void XcodeGenerator::addProduct(const Project &project, const ProductData &product, const InstallOptions &installOptions, PBXProject &xcodeProject, PBXGroup *xcodeGroup)
{
    PBXTarget *xcodeTarget = xcodeTargetForProduct(&xcodeProject, product);
    xcodeTarget->setName(product.name());
    xcodeTarget->setProductType(xcodeProductType(product));

    PBXLegacyTarget *xcodeLegacyTarget = dynamic_cast<PBXLegacyTarget *>(xcodeTarget);
    if (xcodeLegacyTarget) {
        xcodeLegacyTarget->setBuildWorkingDirectory(XcodeGenerator::xcodeInsertPlaceholdersInValue(project, xcodeProject.projectDirPath()).toString());
        xcodeLegacyTarget->setBuildToolPath(QLatin1String("/bin/bash"));

        QStringList buildArguments;
        buildArguments << QLatin1String("\"$(PROJECT_FILE_PATH)\"/") + kQbsRunnerFileName;
        buildArguments << QLatin1String("\"$(ACTION)\"");
        buildArguments << QLatin1String("-f");

        // Can't use xcodeInsertPlaceholdersInValue because we want to do a little acrobatics to get quoting 100% right...
        const QDir sourceDirectory(xcodeProject.projectDirPath());
        QString resultingFilePath = m_qbsProjectFile.absoluteFilePath();
        if (resultingFilePath.startsWith(sourceDirectory.absolutePath())) {
            resultingFilePath.replace(0, sourceDirectory.absolutePath().size(), QString());
            buildArguments << QLatin1String("\"$(SRCROOT)\"") + shellQuote(resultingFilePath);
        } else {
            buildArguments << shellQuote(resultingFilePath);
        }

        buildArguments << QLatin1String("-d");
        buildArguments << QLatin1String("\"$(CONFIGURATION_BUILD_DIR)\"");
        buildArguments << QLatin1String("-p");
        buildArguments << QLatin1String("\"$(TARGET_NAME)\""); // target and product name are opposites in qbs and Xcode
        buildArguments << QLatin1String("\"$(QBS_BUILD_VARIANT)\"");
        buildArguments << QLatin1String("\"profile:$(QBS_PROFILE)\"");

        xcodeLegacyTarget->setBuildArgumentsString(buildArguments.join(QLatin1String(" ")));
        xcodeLegacyTarget->setPassBuildSettingsInEnvironment(true);
    }

    XCBuildConfiguration *xcconfig = xcodeTarget->buildConfigurationList()->addBuildConfiguration(xcodeConfigurationName(product));
    xcodeTarget->buildConfigurationList()->setDefaultBuildConfiguration(xcconfig->name());

    // TODO: If a product with the same name was already added, don't add its files AGAIN
    // Create a root PBX group for the target's files and other groups
    PBXGroup *xcodeProductGroup = m_productGroups.value(product.name());
    const bool productFilesAlreadyAdded = !!xcodeProductGroup;
    if (!productFilesAlreadyAdded) {
        m_productGroups.insert(product.name(), xcodeProductGroup = PBXGroup::groupWithName(product.name(), xcodeGroup));

        // Add the qbs project file for this product
        foreach (PBXFileReference *ref, xcodeProductGroup->addFiles(QStringList() << makePathRelativeTo(product.location().filePath(), xcodeProject.projectDirPath()), false, false)) {
            if (!ref->path().startsWith(QLatin1Char('/'))) {
                ref->setSourceTree(PBX::Group);
            }
        }
    }

    QSet<QString> excludedSources;

    // Set up all the groups
    foreach (const GroupData &groupData, product.groups())
    {
        PBXGroup *group = xcodeProductGroup;
        if (groupData.name() == product.name()) {
            // This group has the same name as the product itself so we'll just add the files
            // directly to the "product" group we created earlier instead of creating a new one
            foreach (const XcodeBuildSettingMapping &setting, XcodeBuildSettingMapping::defaultBuildSettingsMap()) {
                setting.applyToConfiguration(xcconfig, project, product, groupData);
            }

            xcodeTarget->setProductName(xcodeTarget->buildSetting(QLatin1String("PRODUCT_NAME"), buildVariant(product)).toString());
            xcconfig->setBuildSetting(QLatin1String("QBS_PROFILE"), product.profile());
            xcconfig->setBuildSetting(QLatin1String("QBS_BUILD_VARIANT"), buildVariant(product));
        } else {
            // Create a group for this group
            group = PBXGroup::groupWithName(groupData.name(), xcodeProductGroup);
        }

        // Add the source files to the group
        group->setSourceTree(PBX::Group);
        QList<PBXFileReference *> fileRefs;
        if (!productFilesAlreadyAdded)
            fileRefs = addGroupFiles(&xcodeProject, group, groupData);

        // If the group is enabled, then we'll actually BUILD them in addition to merely listing them
        if (groupData.isEnabled()) {
            foreach (PBXFileReference *ref, fileRefs) {
                // Skip header files... wish there was a better way
                if (ref->path().endsWith(QLatin1String(".h")) ||
                    ref->path().endsWith(QLatin1String(".hh"))) {
                    continue;
                }

                xcodeTarget->defaultSourceCodeBuildPhase()->addReference(ref);
            }
        } else {
            foreach (PBXFileReference *ref, fileRefs)
                excludedSources.insert(ref->path());
        }
    }

    xcconfig->setBuildSetting(QLatin1String("EXCLUDED_SOURCE_FILE_NAMES"), flattenXcconfigList(QStringList(excludedSources.toList())));

    QString sysroot = product.moduleProperties().getModuleProperty(QLatin1String("cpp"), QLatin1String("sysroot")).toString();
    QStringList frameworkPaths;
    frameworkPaths += product.moduleProperties().getModulePropertiesAsStringList(QLatin1String("cpp"), QLatin1String("frameworkPaths"));
    frameworkPaths += product.moduleProperties().getModulePropertiesAsStringList(QLatin1String("cpp"), QLatin1String("systemFrameworkPaths"));
    frameworkPaths += sysroot + QLatin1String("/Library/Frameworks");
    frameworkPaths += sysroot + QLatin1String("/System/Library/Frameworks");
    frameworkPaths += QLatin1String("/Library/Frameworks");
    frameworkPaths += QLatin1String("/System/Library/Frameworks");
    const QStringList frameworkNames = product.moduleProperties().getModulePropertiesAsStringList(QLatin1String("cpp"), QLatin1String("frameworks"));
    const QStringList weakFrameworkNames = product.moduleProperties().getModulePropertiesAsStringList(QLatin1String("cpp"), QLatin1String("weakFrameworks"));

    // TODO TODO TODO dynamicLibraries and weakDynamicLibraries?

    // A framework can't be strongly AND weakly linked... TODO: fail more gracefully
    QBS_CHECK(frameworkNames.toSet().intersect(weakFrameworkNames.toSet()).isEmpty());
    const QStringList allFrameworkNames = QStringList() << frameworkNames << weakFrameworkNames;

    Q_FOREACH (const QString &frameworkName, allFrameworkNames) {
        // Whether a full path equivalent for the framework was found
        bool foundFramework = false;

        Q_FOREACH (const QString &frameworkPath, frameworkPaths) {
            QString candidateFrameworkPath = QFileInfo(frameworkPath + QLatin1String("/") + frameworkName + QLatin1String(".framework")).canonicalFilePath();
            if (QFileInfo(candidateFrameworkPath).exists()) {
                QBS_CHECK(!candidateFrameworkPath.isEmpty());

                const QString sysrootedCandidateFrameworkPath = candidateFrameworkPath.mid(sysroot.size() + 1 /* for the path separator */);

                // Check for an existing reference (could happen if a framework is specified multiple times)
                PBXFileReference *existingReference = Q_NULLPTR;
                Q_FOREACH (PBXReference *ref, xcodeProject.frameworksRefGroup()->children()) {
                    if (PBXFileReference *fileRef = dynamic_cast<PBXFileReference *>(ref)) {
                        if ((fileRef->sourceTree() == PBX::SdkRoot && fileRef->path() == sysrootedCandidateFrameworkPath) ||
                                (fileRef->sourceTree() == PBX::Absolute && fileRef->path() == candidateFrameworkPath)) {
                            existingReference = fileRef;
                            break;
                        }
                    }
                }

                QList<PBXFileReference *> refs;

                // Create a new file reference since one wasn't already found in the Frameworks group
                if (!existingReference) {
                    refs = xcodeProject.frameworksRefGroup()->addFiles(QStringList() << candidateFrameworkPath, false, false);
                    Q_FOREACH (PBXFileReference *frameworkRef, refs) {
                        if (!sysroot.isEmpty() && frameworkRef->path().startsWith(sysroot)) {
                            frameworkRef->setPath(sysrootedCandidateFrameworkPath);
                            frameworkRef->setSourceTree(PBX::SdkRoot);
                        }
                    }
                } else {
                    refs.append(existingReference);
                }

                // Add the framework to the Frameworks build phase
                Q_FOREACH (PBXFileReference *frameworkRef, refs) {
                    xcodeTarget->defaultFrameworksBuildPhase()->addReference(frameworkRef);
                }

                foundFramework = true;
                break;
            }
        }

        // Not a normal case, and will cause a build failure, but is better than silently ignoring it...
        if (!foundFramework) {
            Q_FOREACH (PBXFileReference *frameworkRef, xcodeProject.frameworksRefGroup()->addFiles(QStringList() << frameworkName + QLatin1String(".framework"), false, false)) {
                QBS_CHECK(!frameworkRef->path().isEmpty());
                if (PBXBuildFile *frameworkBuildRef = xcodeTarget->defaultFrameworksBuildPhase()->addReference(frameworkRef)) {
                    const bool isWeaklyLinked = weakFrameworkNames.contains(frameworkName);
                    frameworkBuildRef->setWeakLinkedLibrary(isWeaklyLinked);
                }
            }
        }
    }

    // Add a reference for the main target artifact in root "Products" group
    TargetArtifact artifact = runnableOrPrimaryTargetArtifactForProduct(product);
    QString realArtifactPath = installedFilePathForTargetArtifact(project, product, installOptions, artifact);
    if (!realArtifactPath.isEmpty()) {
        // "simple" generator
        if (!xcodeLegacyTarget) {
            // Chop off the Qbs build dir from the artifact path, because they will actually end
            // up in Xcode's build directory, not the build directory Qbs would use
            const QString buildDirectory = project.projectData().buildDirectory();
            if (realArtifactPath.startsWith(buildDirectory))
                realArtifactPath.remove(0, buildDirectory.size());

            if (realArtifactPath.startsWith(QLatin1Char('/')))
                realArtifactPath.remove(0, 1);
        } else {
            realArtifactPath = xcodeInsertPlaceholdersInValue(project, realArtifactPath).toString();
        }

        QBS_CHECK(!realArtifactPath.isEmpty());

        QList<PBXFileReference *> refs = xcodeProject.productRefGroup()->addFiles(QStringList() << realArtifactPath, false, false);
        foreach (PBXFileReference *ref, refs)
        {
            if (!xcodeLegacyTarget) {
                ref->setSourceTree(PBX::BuildProductDir);

                // This causes Xcode to force the file reference to a relative path relative to the build products dir
                // Therefore, we must symlink artifacts in the Xcode build directory to the ones in the qbs build directory
                // NOTE: This will crash Xcode if set for a PBXLegacyTarget (claiming an empty string global hex ID)
                xcodeTarget->setProductReference(ref);
            }

            // TODO: Technically this should be composed of other variables based on the product type
            // $(WRAPPER_NAME) for bundles, $(EXECUTABLE_NAME) otherwise?
            xcconfig->setBuildSetting(QLatin1String("FULL_PRODUCT_NAME"), QFileInfo(realArtifactPath).fileName());
            xcconfig->setBuildSetting(QLatin1String("QBS_TARGET_PATH"), xcodeInsertPlaceholdersInValue(project, realArtifactPath));
        }
    }

    xcodeProject.addTarget(xcodeTarget);
}

QList<PBXFileReference *> XcodeGenerator::addGroupFiles(PBXProject *xcodeProject, PBXGroup *xcodeGroup, const qbs::GroupData &groupData)
{
    QStringList relativeFilePaths;
    foreach (QString filePath, groupData.allFilePaths()) {
        relativeFilePaths += makePathRelativeTo(filePath, xcodeProject->projectDirPath());
    }

    // Add all the files in the group as file references so they show up in the project tree
    QList<PBXFileReference *> fileRefs = xcodeGroup->addFiles(relativeFilePaths, false, false);
    foreach (PBXFileReference *ref, fileRefs) {
        if (!ref->path().startsWith(QLatin1Char('/'))) {
            ref->setSourceTree(PBX::Group);
        }
    }

    return fileRefs;
}

PBXTarget *XcodeGenerator::xcodeTargetForProduct(PBXProject *xcodeProject, const ProductData &product)
{
    Q_UNUSED(xcodeProject);
    PBXTarget *target = xcodeProject->targetNamed(product.name());

    // Native targets without a type won't build, so use a legacy target in this case
    if (!target && xcodeProductType(product) == 0) {
        return new PBXLegacyTarget(xcodeProject);
    }

    return target;
}

/*!
 * Returns the Xcode product type identifier corresponding to the Qbs product type name.
 */
PBX::PBXProductType XcodeGenerator::xcodeProductType(const ProductData &product)
{
    const QStringList productType = product.type();
    const bool isBundle = product.moduleProperties().getModuleProperty(QLatin1String("bundle"), QLatin1String("isBundle")).toBool();

    if (productType.contains(QLatin1String("application")))
        return isBundle ? PBX::Application : PBX::Tool;
    if (productType.contains(QLatin1String("dynamiclibrary")))
        return isBundle ? PBX::Framework : PBX::LibraryDynamic;
    if (productType.contains(QLatin1String("staticlibrary")))
        return isBundle ? PBX::StaticFramework : PBX::LibraryStatic;
    if (productType.contains(QLatin1String("bundle")))
        return PBX::Bundle;

    return (PBX::PBXProductType)0;
}

static QString _xcodeConfigurationName(QString variantName, const QString &profileName, bool includeProfileName)
{
    QBS_CHECK(!variantName.isEmpty());
    variantName.replace(0, 1, variantName[0].toUpper());
    if (includeProfileName) {
        variantName += QLatin1String("_") + profileName;
    }
    return variantName;
}

QString XcodeGenerator::xcodeConfigurationName(const Project &project)
{
    return _xcodeConfigurationName(buildVariant(project), project.profile(), m_multipleProfiles);
}

QString XcodeGenerator::xcodeConfigurationName(const ProductData &product)
{
    return _xcodeConfigurationName(buildVariant(product), product.profile(), m_multipleProfiles);
}

/*!
 * \brief If the value is a path or list of paths,
 * returns it with the prefixes of paths replaced by $(SRCROOT), $(PROJECT_FILE_PATH)/.., etc.,
 * as appropriate.
 * If the value is not a path, returns the original value unmodified.
 */
QVariant XcodeGenerator::xcodeInsertPlaceholdersInValue(const qbs::Project &project, const QVariant &variant)
{
    if (variant.type() == QVariant::List) {
        QVariantList list;
        foreach (const QVariant &v, variant.toList())
            list.append(xcodeInsertPlaceholdersInValue(project, v));
        return list;
    }

    // If the value is a path and it begins with the value of SRCROOT or PROJECT_FILE_PATH/.., replace it
    if (variant.type() == QVariant::String) {
        // This should end up being the same as PBXProject::projectDirPath(), but if projectDirPath
        // is the empty string (for example, when the xcodeproj resides in the qbs source directory,
        // things would break, so prefer the Qbs value (plus, it saves a parameter to this function)
        const QDir sourceDirectory = QFileInfo(project.projectData().location().filePath()).absolutePath();
        if (variant.toString().startsWith(sourceDirectory.absolutePath()))
            return variant.toString().replace(0, sourceDirectory.absolutePath().size(), QLatin1String("$(SRCROOT)"));

        const QDir buildDirectory(project.projectData().buildDirectory());
        Q_ASSERT(buildDirectory.isAbsolute() && buildDirectory.exists());
        if (variant.toString().startsWith(buildDirectory.absolutePath()))
            return variant.toString().replace(0, buildDirectory.absolutePath().size(), QLatin1String("$(QBS_BUILD_DIRECTORY)/$(QBS_PROFILE)-$(QBS_BUILD_VARIANT)"));
    }

    return variant;
}

}

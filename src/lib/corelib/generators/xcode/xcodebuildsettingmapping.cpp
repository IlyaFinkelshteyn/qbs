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
#include "pbx.h"

#include <logging/translator.h>

#include <QDebug>

namespace qbs {

using namespace Internal;

static QVariant _xcodeTargetedDeviceFamily(const Project &project, const ProductData &product, const GroupData &group, const QString &moduleName);

static QVariant xcodeExecutablePrefix(const qbs::Project &project, const ProductData &product, const GroupData &group);
static QVariant xcodeExecutableSuffix(const qbs::Project &project, const ProductData &product, const GroupData &group);
static QVariant xcodeInfoPlistOutputEncoding(const qbs::Project &project, const ProductData &product, const GroupData &group);
static QVariant xcodeArchitecture(const qbs::Project &project, const ProductData &product, const GroupData &group);
static QVariant xcodePreprocessorDefinitions(const qbs::Project &project, const ProductData &product, const GroupData &group);
static QVariant xcodeResourcesTargetedDeviceFamily(const Project &project, const ProductData &product, const GroupData &group);
static QVariant xcodeSdkRoot(const qbs::Project &project, const ProductData &product, const GroupData &group);
static QVariant xcodeSupportedPlatforms(const qbs::Project &project, const ProductData &product, const GroupData &group);
static QVariant xcodeTargetedDeviceFamily(const Project &project, const ProductData &product, const GroupData &group);
static QVariant xcodeDylibVersion(const qbs::Project &project, const ProductData &product, const GroupData &group);
static QVariant xcodeProductName(const qbs::Project &project, const ProductData &product, const GroupData &group);
static QVariant xcodeUseHeadermap(const qbs::Project &project, const ProductData &product, const GroupData &group);

static QVariant qbsProjectModuleProperty(const QString &qbsModuleName, const QString &qbsPropertyName, const qbs::Project &project)
{
    QMapIterator<QString, QVariant> module(project.projectConfiguration());
    while (module.hasNext()) {
        module.next();
        QString qbsModName = qbsModuleName;
        if (module.key() == qbsModName.replace(QLatin1String("."), QLatin1String("/"))) {
            QMapIterator<QString, QVariant> property(module.value().toMap());
            while (property.hasNext()) {
                property.next();
                if (qbsPropertyName == property.key()) {
                    return property.value();
                }
            }
        }
    }

    return QVariant();
}

static QVariant qbsModuleProperty(const QString &qbsModuleName, const QString &qbsPropertyName, const qbs::Project &project, const qbs::ProductData &product, const qbs::GroupData &group)
{
    if (product.isValid()) {
        return (group.isValid() ? group.properties() : product.moduleProperties()).getModuleProperty(qbsModuleName, qbsPropertyName);
    } else {
        return qbsProjectModuleProperty(qbsModuleName, qbsPropertyName, project);
    }
    return QVariant();
}

static QVariantList qbsModuleProperties(const QString &qbsModuleName, const QString &qbsPropertyName, const qbs::Project &project, const qbs::ProductData &product, const qbs::GroupData &group)
{
    if (product.isValid()) {
        return (group.isValid() ? group.properties() : product.moduleProperties()).getModuleProperties(qbsModuleName, qbsPropertyName);
    } else {
        return qbsProjectModuleProperty(qbsModuleName, qbsPropertyName, project).toList();
    }
    return QVariantList();
}

void XcodeBuildSettingMapping::applyToProject(PBXProject *xcodeProject,
                                              const qbs::Project &project) const {
    applyToConfiguration(xcodeProject->buildConfigurationList(), project);
}

void XcodeBuildSettingMapping::applyToTarget(PBXTarget *xcodeTarget,
                                             const qbs::Project &project,
                                             const qbs::ProductData &productData,
                                             const qbs::GroupData &groupData) const {
    applyToConfiguration(xcodeTarget->buildConfigurationList(), project, productData, groupData);
}

void XcodeBuildSettingMapping::applyToConfiguration(XCConfigurationList *xcodeConfigurationList,
                                                    const Project &project,
                                                    const ProductData &productData,
                                                    const GroupData &groupData) const {
    foreach (XCBuildConfiguration *xcodeConfiguration, xcodeConfigurationList->buildConfigurations())
        applyToConfiguration(xcodeConfiguration, project, productData, groupData);
}

void XcodeBuildSettingMapping::applyToConfiguration(XCBuildConfiguration *xcodeConfiguration,
                                                    const qbs::Project &project,
                                                    const qbs::ProductData &productData,
                                                    const qbs::GroupData &groupData) const {
    QVariant xcodeValue;
    if (function) {
        xcodeValue = function(project, productData, groupData);
    } else if (isList) {
        QVariantList list;

        foreach (const QString &qbsName, qbsNames) {
            list << qbsModuleProperties(qbsModule, qbsName, project, productData, groupData);
        }

        if (!list.isEmpty()) {
            list.append(QLatin1String("$(inherited)"));
            xcodeValue = list;
        }
    } else {
        xcodeValue = qbsModuleProperty(qbsModule, qbsNames.first(), project, productData, groupData);
    }

    // TODO: Figure out what Xcode *really* does... I think it's this
    bool shouldSerializeAsList = serializedAsList;
    if (xcodeValue.type() == QVariant::List || xcodeValue.type() == QVariant::StringList) {
        const QStringList l = xcodeValue.toStringList();
        if (l.size() <= 1) {
            shouldSerializeAsList = false; // || containsQuotedStrings ??? lineLength?
        }
    }

    if (!shouldSerializeAsList && (xcodeValue.type() == QVariant::List
                              || xcodeValue.type() == QVariant::StringList)) {
        xcodeValue = xcodeValue.toStringList().join(QLatin1Char(' '));
    }

    if (!xcodeValue.isNull()) {
        xcodeValue = XcodeGenerator::xcodeInsertPlaceholdersInValue(project, xcodeValue);
        xcodeConfiguration->setBuildSetting(xcodeName, xcodeValue);
    }
}

/*!
 * \brief Mapping of Xcode build settings to qbs module and property names or functions.
 */
QList<XcodeBuildSettingMapping> XcodeBuildSettingMapping::defaultBuildSettingsMap()
{
    static const QString cpp = QLatin1String("cpp");
    static const QString xcode = QLatin1String("xcode");

    QList<XcodeBuildSettingMapping> settings;
    settings += XcodeBuildSettingMapping(QLatin1String("APPLICATION_EXTENSION_API_ONLY"), cpp, QLatin1String("requireAppExtensionSafeApi"));
    settings += XcodeBuildSettingMapping(QLatin1String("ARCHS"), &xcodeArchitecture, true);
    settings += XcodeBuildSettingMapping(QLatin1String("CLANG_ENABLE_OBJC_ARC"), cpp, QLatin1String("automaticReferenceCounting"));
    settings += XcodeBuildSettingMapping(QLatin1String("CODE_SIGN_IDENTITY"), xcode, QLatin1String("signingIdentity"));
    settings += XcodeBuildSettingMapping(QLatin1String("CREATE_INFOPLIST_SECTION_IN_BINARY"), cpp, QLatin1String("embedInfoPlist"));
    settings += XcodeBuildSettingMapping(QLatin1String("DYLIB_CURRENT_VERSION"), &xcodeDylibVersion, false);
    settings += XcodeBuildSettingMapping(QLatin1String("DYLIB_INSTALL_NAME_BASE"), cpp, QLatin1String("installNamePrefix"));
    settings += XcodeBuildSettingMapping(QLatin1String("EXECUTABLE_EXTENSION"), &xcodeExecutableSuffix, false);
    settings += XcodeBuildSettingMapping(QLatin1String("EXECUTABLE_PREFIX"), &xcodeExecutablePrefix, false);
    settings += XcodeBuildSettingMapping(QLatin1String("FRAMEWORK_SEARCH_PATHS"), cpp, QStringList() << QLatin1String("frameworkPaths"));
    settings += XcodeBuildSettingMapping(QLatin1String("GCC_PREFIX_HEADER"), cpp, QLatin1String("precompiledHeader"));
    settings += XcodeBuildSettingMapping(QLatin1String("GCC_PREPROCESSOR_DEFINITIONS"), &xcodePreprocessorDefinitions, true);
    settings += XcodeBuildSettingMapping(QLatin1String("HEADER_SEARCH_PATHS"), cpp, QStringList() << QLatin1String("includePaths"));
    settings += XcodeBuildSettingMapping(QLatin1String("INFOPLIST_EXPAND_BUILD_SETTINGS"), cpp, QLatin1String("processInfoPlist"));
    settings += XcodeBuildSettingMapping(QLatin1String("INFOPLIST_FILE"), cpp, QLatin1String("infoPlistFile"));
    settings += XcodeBuildSettingMapping(QLatin1String("INFOPLIST_OUTPUT_FORMAT"), &xcodeInfoPlistOutputEncoding, false);
    settings += XcodeBuildSettingMapping(QLatin1String("IPHONEOS_DEPLOYMENT_TARGET"), cpp, QLatin1String("minimumIosVersion"));
    settings += XcodeBuildSettingMapping(QLatin1String("LD_RUNPATH_SEARCH_PATHS"), cpp, QStringList() << QLatin1String("rpaths"), false);
    settings += XcodeBuildSettingMapping(QLatin1String("LIBRARY_SEARCH_PATHS"), cpp, QStringList() << QLatin1String("libraryPaths"));
    settings += XcodeBuildSettingMapping(QLatin1String("MACOSX_DEPLOYMENT_TARGET"), cpp, QLatin1String("minimumOsxVersion"));
    settings += XcodeBuildSettingMapping(QLatin1String("OTHER_CFLAGS"), cpp, QStringList() << QLatin1String("cFlags") << QLatin1String("objcFlags"));
    settings += XcodeBuildSettingMapping(QLatin1String("OTHER_CODE_SIGN_FLAGS"), xcode, QStringList() << QLatin1String("codesignFlags"));
    settings += XcodeBuildSettingMapping(QLatin1String("OTHER_CPLUSPLUSFLAGS"), cpp, QStringList() << QLatin1String("cxxFlags") << QLatin1String("objcxxFlags"));
    settings += XcodeBuildSettingMapping(QLatin1String("OTHER_LDFLAGS"), cpp, QStringList() << QLatin1String("linkerFlags"));
    settings += XcodeBuildSettingMapping(QLatin1String("PRODUCT_NAME"), &xcodeProductName, false);
    settings += XcodeBuildSettingMapping(QLatin1String("PROVISIONING_PROFILE"), xcode, QLatin1String("provisioningProfile"));
    settings += XcodeBuildSettingMapping(QLatin1String("RESOURCES_TARGETED_DEVICE_FAMILY"), &xcodeResourcesTargetedDeviceFamily, false);
    settings += XcodeBuildSettingMapping(QLatin1String("SDKROOT"), &xcodeSdkRoot, false);
    settings += XcodeBuildSettingMapping(QLatin1String("SUPPORTED_PLATFORMS"), &xcodeSupportedPlatforms, false);
    settings += XcodeBuildSettingMapping(QLatin1String("TARGETED_DEVICE_FAMILY"), &xcodeTargetedDeviceFamily, false);
    settings += XcodeBuildSettingMapping(QLatin1String("USE_HEADERMAP"), &xcodeUseHeadermap, false);
    settings += XcodeBuildSettingMapping(QLatin1String("WATCHOS_DEPLOYMENT_TARGET"), cpp, QLatin1String("minimumWatchosVersion"));
    return settings;
}

QVariant xcodeExecutablePrefix(const Project &project, const ProductData &product, const GroupData &group)
{
    switch (XcodeGenerator::xcodeProductType(product))
    {
    case PBX::Application:
    case PBX::Tool:
    case PBX::Bundle:
    case PBX::Framework:
        return qbsModuleProperty(QLatin1String("cpp"), QLatin1String("executablePrefix"), project, product, group).toString();
    case PBX::LibraryDynamic:
        return qbsModuleProperty(QLatin1String("cpp"), QLatin1String("dynamicLibraryPrefix"), project, product, group).toString();
    case PBX::LibraryStatic:
        return qbsModuleProperty(QLatin1String("cpp"), QLatin1String("staticLibraryPrefix"), project, product, group).toString();
    default:
        return QString();
    }
}

QVariant xcodeExecutableSuffix(const Project &project, const ProductData &product, const GroupData &group)
{
    QString suffix;
    switch (XcodeGenerator::xcodeProductType(product))
    {
    case PBX::Application:
    case PBX::Tool:
    case PBX::Bundle:
    case PBX::Framework:
        suffix = qbsModuleProperty(QLatin1String("cpp"), QLatin1String("executableSuffix"), project, product, group).toString();
        break;
    case PBX::LibraryDynamic:
        suffix = qbsModuleProperty(QLatin1String("cpp"), QLatin1String("dynamicLibrarySuffix"), project, product, group).toString();
        break;
    case PBX::LibraryStatic:
        suffix = qbsModuleProperty(QLatin1String("cpp"), QLatin1String("staticLibrarySuffix"), project, product, group).toString();
        break;
    default:
        break;
    }

    if (suffix.startsWith(QLatin1Char('.')))
        suffix.remove(0, 1);

    return suffix;
}

QVariant xcodeInfoPlistOutputEncoding(const Project &project, const ProductData &product, const GroupData &group)
{
    const QString format = qbsModuleProperty(QLatin1String("cpp"), QLatin1String("infoPlistFormat"), project, product, group).toString();
    if (format == QLatin1String("xml1"))
        return QLatin1String("XML");
    else if (format == QLatin1String("binary1"))
        return QLatin1String("binary");

    // If we don't know what the format is, return the original string (which may be invalid)
    // "same-as-input" is valid
    return format;
}

QVariant xcodeArchitecture(const Project &project, const ProductData &product, const GroupData &group)
{
    const QString architecture = qbsModuleProperty(QLatin1String("qbs"), QLatin1String("architecture"), project, product, group).toString();
    if (architecture == QLatin1String("x86"))
        return QLatin1String("i386");
    return architecture;
}

QVariant xcodePreprocessorDefinitions(const qbs::Project &project, const ProductData &product, const GroupData &group)
{
    QStringList defs;
    Q_FOREACH (const QVariant &def, qbsModuleProperties(QLatin1String("cpp"), QLatin1String("defines"), project, product, group)) {
        // Preprocessor definitions need to be double escaped since Xcode ignores quoted strings
        defs.append(def.toString().replace(QLatin1String("\""), QLatin1String("\\\"")));
    }
    return defs;
}

QVariant xcodeSdkRoot(const Project &project, const ProductData &product, const GroupData &group)
{
    const QString sdkName = qbsModuleProperty(QLatin1String("xcode"), QLatin1String("sdkName"), project, product, group).toString();
    if (!sdkName.isEmpty())
        return sdkName;

    const QString sysroot = qbsModuleProperty(QLatin1String("qbs"), QLatin1String("sysroot"), project, product, group).toString();
    if (!sysroot.isEmpty())
        return sysroot;

    const QVariantList targetOS = qbsModuleProperties(QLatin1String("qbs"), QLatin1String("targetOS"), project, product, group);
    if (targetOS.contains(QLatin1String("osx")))
        return QLatin1String("macosx");
    else if (targetOS.contains(QLatin1String("ios")))
        return QLatin1String("iphoneos");
    else if (targetOS.contains(QLatin1String("watchos")))
        return QLatin1String("watchos");
    else
        throw ErrorInfo(Tr::tr("Unsupported platform for Xcode project"));
}

QVariant xcodeSupportedPlatforms(const Project &project, const ProductData &product, const GroupData &group)
{
    QStringList supportedPlatforms;
    const QVariantList targetOS = qbsModuleProperties(QLatin1String("qbs"), QLatin1String("targetOS"), project, product, group);
    if (targetOS.contains(QLatin1String("osx"))) {
        supportedPlatforms.append(QLatin1String("macosx"));
    } else if (targetOS.contains(QLatin1String("ios"))) {
        if (targetOS.contains(QLatin1String("ios-simulator")))
            supportedPlatforms.append(QLatin1String("iphonesimulator"));
        supportedPlatforms.append(QLatin1String("iphoneos"));
    } else if (targetOS.contains(QLatin1String("watchos"))) {
        if (targetOS.contains(QLatin1String("watchos-simulator")))
            supportedPlatforms.append(QLatin1String("watchos-simulator"));
        supportedPlatforms.append(QLatin1String("watchos"));
    }
    return supportedPlatforms;
}

static inline int targetDeviceFamilyIdentifierFromName(const QString &name)
{
    if (name == QStringLiteral("iphone"))
        return 1;
    if (name == QStringLiteral("ipad"))
        return 2;
    if (name == QStringLiteral("tv"))
        return 3;
    if (name == QStringLiteral("watch"))
        return 4;
    if (name == QStringLiteral("car"))
        return 5;
    return 0;
}

QVariant _xcodeTargetedDeviceFamily(const Project &project, const ProductData &product, const GroupData &group, const QString &moduleName)
{
    QStringList targetedDeviceFamily;
    const QVariantList targetDevices = qbsModuleProperties(moduleName, QLatin1String("targetDevices"), project, product, group);
    Q_FOREACH (const QVariant &targetDevice, targetDevices) {
        int devId = targetDeviceFamilyIdentifierFromName(targetDevice.toString());
        if (devId != 0) {
            targetedDeviceFamily.append(QString::number(devId));
        }
    }
    return targetedDeviceFamily.join(QLatin1Char(','));
}

QVariant xcodeResourcesTargetedDeviceFamily(const Project &project, const ProductData &product, const GroupData &group)
{
    return _xcodeTargetedDeviceFamily(project, product, group, QLatin1String("ib"));
}

QVariant xcodeTargetedDeviceFamily(const Project &project, const ProductData &product, const GroupData &group)
{
    return _xcodeTargetedDeviceFamily(project, product, group, QLatin1String("xcode"));
}

QVariant xcodeDylibVersion(const Project &project, const ProductData &product, const GroupData &group)
{
    Q_UNUSED(project);
    Q_UNUSED(group);
    if (product.type().contains(QLatin1String("dynamiclibrary")))
        return product.version();
    return QString();
}

QVariant xcodeProductName(const Project &project, const ProductData &product, const GroupData &group)
{
    Q_UNUSED(project);
    Q_UNUSED(group);
    if (!product.targetName().isEmpty())
        return product.targetName();
    return product.name();
}

QVariant xcodeUseHeadermap(const Project &, const ProductData &, const GroupData &)
{
    return QVariant(false); // We never want to use headermaps in qbs-generated projects
}

} // namespace qbs

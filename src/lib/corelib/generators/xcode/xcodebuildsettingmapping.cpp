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
#include "xcodegenerator.h"
#include "xcodegeneratorutils.h"

#include <qbs.h>
#include "pbx/pbx.h"

#include <logging/translator.h>

#include <QDebug>

namespace qbs {

using namespace Internal;

static QVariant _xcodeTargetedDeviceFamily(const Project &project, const ProductData &product, const GroupData &group, const QString &moduleName);

static QVariant xcodeDebugInformationFormat(const GeneratableProject &p, const qbs::Project &project, const GeneratableProductData &gproduct, const ProductData &product, const GroupData &group);
static QVariant xcodeExecutablePrefix(const GeneratableProject &p, const qbs::Project &project, const GeneratableProductData &gproduct, const ProductData &product, const GroupData &group);
static QVariant xcodeExecutableSuffix(const GeneratableProject &p, const qbs::Project &project, const GeneratableProductData &gproduct, const ProductData &product, const GroupData &group);
static QVariant xcodeInfoPlistOutputEncoding(const GeneratableProject &p, const qbs::Project &project, const GeneratableProductData &gproduct, const ProductData &product, const GroupData &group);
static QVariant xcodeArchitecture(const GeneratableProject &p, const qbs::Project &project, const GeneratableProductData &gproduct, const ProductData &product, const GroupData &group);
static QVariant xcodePreprocessorDefinitions(const GeneratableProject &p, const qbs::Project &project, const GeneratableProductData &gproduct, const ProductData &product, const GroupData &group);
static QVariant xcodeResourcesTargetedDeviceFamily(const GeneratableProject &p, const Project &project, const GeneratableProductData &gproduct, const ProductData &product, const GroupData &group);
static QVariant xcodeSdkRoot(const GeneratableProject &p, const qbs::Project &project, const GeneratableProductData &gproduct, const ProductData &product, const GroupData &group);
static QVariant xcodeSupportedPlatforms(const GeneratableProject &p, const qbs::Project &project, const GeneratableProductData &gproduct, const ProductData &product, const GroupData &group);
static QVariant xcodeTargetedDeviceFamily(const GeneratableProject &p, const Project &project, const GeneratableProductData &gproduct, const ProductData &product, const GroupData &group);
static QVariant xcodeDylibVersion(const GeneratableProject &p, const qbs::Project &project, const GeneratableProductData &gproduct, const ProductData &product, const GroupData &group);
static QVariant xcodeProductName(const GeneratableProject &p, const qbs::Project &project, const GeneratableProductData &gproduct, const ProductData &product, const GroupData &group);
static QVariant xcodeUseHeadermap(const GeneratableProject &p, const qbs::Project &project, const GeneratableProductData &gproduct, const ProductData &product, const GroupData &group);

static QVariant qbsProjectModuleProperty(const QString &qbsModuleName, const QString &qbsPropertyName, const qbs::Project &project)
{
    QMapIterator<QString, QVariant> module(project.projectConfiguration());
    while (module.hasNext()) {
        module.next();
        QString qbsModName = qbsModuleName;
        if (module.key() == qbsModName.replace(QStringLiteral("."), QStringLiteral("/"))) {
            QMapIterator<QString, QVariant> property(module.value().toMap());
            while (property.hasNext()) {
                property.next();
                if (qbsPropertyName == property.key())
                    return property.value();
            }
        }
    }

    return QVariant();
}

static QVariant qbsModuleProperty(const QString &qbsModuleName,
                                  const QString &qbsPropertyName,
                                  const qbs::Project &project,
                                  const qbs::ProductData &product,
                                  const qbs::GroupData &group)
{
    if (product.isValid())
        return (group.isValid() ? group.properties() : product.moduleProperties()).getModuleProperty(qbsModuleName, qbsPropertyName);
    return qbsProjectModuleProperty(qbsModuleName, qbsPropertyName, project);
}

static QVariantList qbsModuleProperties(const QString &qbsModuleName,
                                        const QString &qbsPropertyName,
                                        const qbs::Project &project,
                                        const qbs::ProductData &product,
                                        const qbs::GroupData &group)
{
    if (product.isValid())
        return (group.isValid() ? group.properties() : product.moduleProperties()).getModuleProperty(qbsModuleName, qbsPropertyName).toList();
    return qbsProjectModuleProperty(qbsModuleName, qbsPropertyName, project).toList();
}

void XcodeBuildSettingMapping::applyToProject(PBXProject *xcodeProject,
                                              const GeneratableProject &p,
                                              const qbs::Project &project) const {
    applyToConfiguration(xcodeProject->buildConfigurationList(), p, project);
}

void XcodeBuildSettingMapping::applyToTarget(PBXTarget *xcodeTarget,
                                             const GeneratableProject &p,
                                             const qbs::Project &project,
                                             const GeneratableProductData &product,
                                             const qbs::ProductData &productData,
                                             const qbs::GroupData &groupData) const {
    applyToConfiguration(xcodeTarget->buildConfigurationList(),
                         p, project, product, productData, groupData);
}

void XcodeBuildSettingMapping::applyToConfiguration(XCConfigurationList *xcodeConfigurationList,
                                                    const GeneratableProject &p,
                                                    const Project &project,
                                                    const GeneratableProductData &product,
                                                    const ProductData &productData,
                                                    const GroupData &groupData) const {
    for (XCBuildConfiguration *xcodeConfiguration : xcodeConfigurationList->buildConfigurations())
        applyToConfiguration(xcodeConfiguration, p, project, product, productData, groupData);
}

void XcodeBuildSettingMapping::applyToConfiguration(XCBuildConfiguration *xcodeConfiguration,
                                                    const GeneratableProject &p,
                                                    const qbs::Project &project,
                                                    const GeneratableProductData &product,
                                                    const qbs::ProductData &productData,
                                                    const qbs::GroupData &groupData) const {
    QVariant xcodeValue;
    if (function) {
        xcodeValue = function(p, project, product, productData, groupData);
    } else if (isList) {
        QVariantList list;

        for (const QString &qbsName : qbsNames)
            list << qbsModuleProperties(qbsModule, qbsName, project, productData, groupData);

        if (!list.isEmpty()) {
            list.append(QStringLiteral("$(inherited)"));
            xcodeValue = list;
        }
    } else {
        xcodeValue = qbsModuleProperty(qbsModule, qbsNames.first(), project, productData, groupData);
    }

    // TODO: Figure out what Xcode *really* does... I think it's this
    bool shouldSerializeAsList = serializedAsList;
    if (xcodeValue.type() == QVariant::List || xcodeValue.type() == QVariant::StringList) {
        const QStringList l = xcodeValue.toStringList();
        if (l.size() <= 1)
            shouldSerializeAsList = false; // || containsQuotedStrings ??? lineLength?
    }

    if (!shouldSerializeAsList && (xcodeValue.type() == QVariant::List
                              || xcodeValue.type() == QVariant::StringList))
        xcodeValue = xcodeValue.toStringList().join(QLatin1Char(' '));

    if (!xcodeValue.isNull()) {
        xcodeValue = XcodeGeneratorUtils::xcodeInsertPlaceholdersInValue(p, xcodeValue);
        xcodeConfiguration->setBuildSetting(xcodeName, xcodeValue);
    }
}

/*!
 * \brief Mapping of Xcode build settings to qbs module and property names or functions.
 */
QList<XcodeBuildSettingMapping> XcodeBuildSettingMapping::defaultBuildSettingsMap()
{
    static const QString bundle = QStringLiteral("bundle");
    static const QString cpp = QStringLiteral("cpp");
    static const QString ib = QStringLiteral("ib");
    static const QString xcode = QStringLiteral("xcode");

    QList<XcodeBuildSettingMapping> settings;
    settings += XcodeBuildSettingMapping(QStringLiteral("APPLICATION_EXTENSION_API_ONLY"), cpp, QStringLiteral("requireAppExtensionSafeApi"));
    settings += XcodeBuildSettingMapping(QStringLiteral("ARCHS"), &xcodeArchitecture, true);
    settings += XcodeBuildSettingMapping(QStringLiteral("ASSETCATALOG_COMPILER_APPICON_NAME"), ib, QStringLiteral("appIconName"));
    settings += XcodeBuildSettingMapping(QStringLiteral("ASSETCATALOG_COMPILER_LAUNCHIMAGE_NAME"), ib, QStringLiteral("launchImageName"));
    settings += XcodeBuildSettingMapping(QStringLiteral("ASSETCATALOG_NOTICES"), ib, QStringLiteral("notices"));
    settings += XcodeBuildSettingMapping(QStringLiteral("ASSETCATALOG_OTHER_FLAGS"), ib, QStringLiteral("flags"));
    settings += XcodeBuildSettingMapping(QStringLiteral("ASSETCATALOG_WARNINGS"), ib, QStringLiteral("warnings"));
    settings += XcodeBuildSettingMapping(QStringLiteral("CLANG_CXX_LANGUAGE_STANDARD"), cpp, QStringLiteral("cxxLanguageVersion"));
    settings += XcodeBuildSettingMapping(QStringLiteral("CLANG_CXX_LIBRARY"), cpp, QStringLiteral("cxxStandardLibrary"));
    settings += XcodeBuildSettingMapping(QStringLiteral("CLANG_ENABLE_OBJC_ARC"), cpp, QStringLiteral("automaticReferenceCounting"));
    settings += XcodeBuildSettingMapping(QStringLiteral("CODE_SIGN_IDENTITY"), xcode, QStringLiteral("signingIdentity"));
    settings += XcodeBuildSettingMapping(QStringLiteral("CREATE_INFOPLIST_SECTION_IN_BINARY"), cpp, QStringLiteral("embedInfoPlist"));
    settings += XcodeBuildSettingMapping(QStringLiteral("DEBUG_INFORMATION_FORMAT"), &xcodeDebugInformationFormat, false);
    settings += XcodeBuildSettingMapping(QStringLiteral("DYLIB_CURRENT_VERSION"), &xcodeDylibVersion, false);
    settings += XcodeBuildSettingMapping(QStringLiteral("DYLIB_INSTALL_NAME_BASE"), cpp, QStringLiteral("installNamePrefix"));
    settings += XcodeBuildSettingMapping(QStringLiteral("EXECUTABLE_EXTENSION"), &xcodeExecutableSuffix, false);
    settings += XcodeBuildSettingMapping(QStringLiteral("EXECUTABLE_PREFIX"), &xcodeExecutablePrefix, false);
    settings += XcodeBuildSettingMapping(QStringLiteral("FRAMEWORK_VERSION"), bundle, QStringLiteral("frameworkVersion"));
    settings += XcodeBuildSettingMapping(QStringLiteral("FRAMEWORK_SEARCH_PATHS"), cpp, QStringList() << QStringLiteral("frameworkPaths"));
    settings += XcodeBuildSettingMapping(QStringLiteral("GENERATE_PKGINFO_FILE"), bundle, QStringLiteral("generatePackageInfo"));
    settings += XcodeBuildSettingMapping(QStringLiteral("GCC_C_LANGUAGE_STANDARD"), cpp, QStringLiteral("cLanguageVersion"));
    settings += XcodeBuildSettingMapping(QStringLiteral("GCC_ENABLE_CPP_EXCEPTIONS"), cpp, QStringLiteral("enableExceptions"));
    settings += XcodeBuildSettingMapping(QStringLiteral("GCC_ENABLE_OBJC_EXCEPTIONS"), cpp, QStringLiteral("enableExceptions"));
    settings += XcodeBuildSettingMapping(QStringLiteral("GCC_ENABLE_CPP_RTTI"), cpp, QStringLiteral("enableRtti"));
    settings += XcodeBuildSettingMapping(QStringLiteral("GCC_GENERATE_DEBUGGING_SYMBOLS"), cpp, QStringLiteral("debugInformation"));
    settings += XcodeBuildSettingMapping(QStringLiteral("GCC_PREFIX_HEADER"), cpp, QStringLiteral("precompiledHeader"));
    settings += XcodeBuildSettingMapping(QStringLiteral("GCC_PREPROCESSOR_DEFINITIONS"), &xcodePreprocessorDefinitions, true);
    settings += XcodeBuildSettingMapping(QStringLiteral("GCC_TREAT_WARNINGS_AS_ERRORS"), cpp, QStringLiteral("treatWarningsAsErrors"));
    settings += XcodeBuildSettingMapping(QStringLiteral("HEADER_SEARCH_PATHS"), cpp, QStringList() << QStringLiteral("includePaths"));
    settings += XcodeBuildSettingMapping(QStringLiteral("IBC_COMPILER_AUTO_ACTIVATE_CUSTOM_FONTS"), ib, QStringLiteral("autoActivateCustomFonts"));
    settings += XcodeBuildSettingMapping(QStringLiteral("IBC_ERRORS"), ib, QStringLiteral("errors"));
    settings += XcodeBuildSettingMapping(QStringLiteral("IBC_FLATTEN_NIBS"), ib, QStringLiteral("flatten"));
    settings += XcodeBuildSettingMapping(QStringLiteral("IBC_NOTICES"), ib, QStringLiteral("notices"));
    settings += XcodeBuildSettingMapping(QStringLiteral("IBC_OTHER_FLAGS"), ib, QStringLiteral("flags"));
    settings += XcodeBuildSettingMapping(QStringLiteral("IBC_WARNINGS"), ib, QStringLiteral("warnings"));
    settings += XcodeBuildSettingMapping(QStringLiteral("IBSC_COMPILER_AUTO_ACTIVATE_CUSTOM_FONTS"), ib, QStringLiteral("autoActivateCustomFonts"));
    settings += XcodeBuildSettingMapping(QStringLiteral("IBSC_ERRORS"), ib, QStringLiteral("errors"));
    settings += XcodeBuildSettingMapping(QStringLiteral("IBSC_FLATTEN_NIBS"), ib, QStringLiteral("flatten"));
    settings += XcodeBuildSettingMapping(QStringLiteral("IBSC_NOTICES"), ib, QStringLiteral("notices"));
    settings += XcodeBuildSettingMapping(QStringLiteral("IBSC_OTHER_FLAGS"), ib, QStringLiteral("flags"));
    settings += XcodeBuildSettingMapping(QStringLiteral("IBSC_WARNINGS"), ib, QStringLiteral("warnings"));
    settings += XcodeBuildSettingMapping(QStringLiteral("INFOPLIST_EXPAND_BUILD_SETTINGS"), cpp, QStringLiteral("processInfoPlist"));
    settings += XcodeBuildSettingMapping(QStringLiteral("INFOPLIST_FILE"), cpp, QStringLiteral("infoPlistFile"));
    settings += XcodeBuildSettingMapping(QStringLiteral("INFOPLIST_OUTPUT_FORMAT"), &xcodeInfoPlistOutputEncoding, false);
    settings += XcodeBuildSettingMapping(QStringLiteral("IPHONEOS_DEPLOYMENT_TARGET"), cpp, QStringLiteral("minimumIosVersion"));
    settings += XcodeBuildSettingMapping(QStringLiteral("LD_RUNPATH_SEARCH_PATHS"), cpp, QStringList() << QStringLiteral("rpaths"), false);
    settings += XcodeBuildSettingMapping(QStringLiteral("LIBRARY_SEARCH_PATHS"), cpp, QStringList() << QStringLiteral("libraryPaths"));
    settings += XcodeBuildSettingMapping(QStringLiteral("MACOSX_DEPLOYMENT_TARGET"), cpp, QStringLiteral("minimumOsxVersion"));
    settings += XcodeBuildSettingMapping(QStringLiteral("OTHER_CFLAGS"), cpp, QStringList() << QStringLiteral("cFlags") << QStringLiteral("objcFlags"));
    settings += XcodeBuildSettingMapping(QStringLiteral("OTHER_CODE_SIGN_FLAGS"), xcode, QStringList() << QStringLiteral("codesignFlags"));
    settings += XcodeBuildSettingMapping(QStringLiteral("OTHER_CPLUSPLUSFLAGS"), cpp, QStringList() << QStringLiteral("cxxFlags") << QStringLiteral("objcxxFlags"));
    settings += XcodeBuildSettingMapping(QStringLiteral("OTHER_LDFLAGS"), cpp, QStringList() << QStringLiteral("linkerFlags"));
    settings += XcodeBuildSettingMapping(QStringLiteral("PRODUCT_BUNDLE_IDENTIFIER"), bundle, QStringLiteral("identifier"));
    settings += XcodeBuildSettingMapping(QStringLiteral("PRODUCT_NAME"), &xcodeProductName, false);
    settings += XcodeBuildSettingMapping(QStringLiteral("PROVISIONING_PROFILE"), xcode, QStringLiteral("provisioningProfile"));
    settings += XcodeBuildSettingMapping(QStringLiteral("RESOURCES_TARGETED_DEVICE_FAMILY"), &xcodeResourcesTargetedDeviceFamily, false);
    settings += XcodeBuildSettingMapping(QStringLiteral("SDKROOT"), &xcodeSdkRoot, false);
    settings += XcodeBuildSettingMapping(QStringLiteral("SUPPORTED_PLATFORMS"), &xcodeSupportedPlatforms, false);
    settings += XcodeBuildSettingMapping(QStringLiteral("TARGETED_DEVICE_FAMILY"), &xcodeTargetedDeviceFamily, false);
    settings += XcodeBuildSettingMapping(QStringLiteral("TVOS_DEPLOYMENT_TARGET"), cpp, QStringLiteral("minimumTvosVersion"));
    settings += XcodeBuildSettingMapping(QStringLiteral("USE_HEADERMAP"), &xcodeUseHeadermap, false);
    settings += XcodeBuildSettingMapping(QStringLiteral("WATCHOS_DEPLOYMENT_TARGET"), cpp, QStringLiteral("minimumWatchosVersion"));
    settings += XcodeBuildSettingMapping(QStringLiteral("WRAPPER_EXTENSION"), bundle, QStringLiteral("extension"));
    return settings;
}

QVariant xcodeDebugInformationFormat(const GeneratableProject &,
                                     const qbs::Project &project,
                                     const GeneratableProductData &,
                                     const ProductData &product,
                                     const GroupData &group)
{
    if (qbsModuleProperty(QStringLiteral("cpp"), QStringLiteral("debugInformation"), project, product, group).toBool()) {
        if (qbsModuleProperty(QStringLiteral("cpp"), QStringLiteral("separateDebugInfo"), project, product, group).toBool())
            return QStringLiteral("dwarf-with-dsym");
        else
            return QStringLiteral("dwarf");
    }
    return QVariant();
}

QVariant xcodeExecutableEdge(const GeneratableProject &,
                             const Project &project,
                             const GeneratableProductData &gproduct,
                             const ProductData &product,
                             const GroupData &group,
                             const QString &edgeName)
{
    const auto type = XcodeGeneratorUtils::xcodeProductType(gproduct);
    if (type == PBX::PBXDynamicLibraryProductType)
        return qbsModuleProperty(QStringLiteral("cpp"), QStringLiteral("dynamicLibrary") + edgeName, project, product, group).toString();
    if (type == PBX::PBXStaticLibraryProductType)
        return qbsModuleProperty(QStringLiteral("cpp"), QStringLiteral("staticLibrary") + edgeName, project, product, group).toString();
    if (type == PBX::PBXApplicationProductType ||
            type == PBX::PBXBundleProductType ||
            type == PBX::PBXFrameworkProductType ||
            type == PBX::PBXShallowBundleProductType ||
            type == PBX::PBXShallowFrameworkProductType ||
            type == PBX::PBXStaticLibraryProductType ||
            type == PBX::PBXToolProductType ||
            type == PBX::XCKernelExtensionProductType ||
            type == PBX::XCShallowKernelExtensionProductType ||
            type == PBX::XCStaticFrameworkProductType ||
            type == PBX::XCApplicationExtensionProductType ||
            type == PBX::XCXPCServiceProductType)
        return qbsModuleProperty(QStringLiteral("cpp"), QStringLiteral("executable") + edgeName, project, product, group).toString();

    return QString();
}

QVariant xcodeExecutablePrefix(const GeneratableProject &p,
                               const Project &project,
                               const GeneratableProductData &gproduct,
                               const ProductData &product,
                               const GroupData &group)
{
    return xcodeExecutableEdge(p, project, gproduct, product, group, QStringLiteral("Prefix"));
}

QVariant xcodeExecutableSuffix(const GeneratableProject &p,
                               const Project &project,
                               const GeneratableProductData &gproduct,
                               const ProductData &product,
                               const GroupData &group)
{
    QVariant suffix = xcodeExecutableEdge(p, project, gproduct, product, group, QStringLiteral("Suffix"));
    if (suffix.type() == QVariant::String && suffix.toString().startsWith(QLatin1Char('.')))
        suffix = suffix.toString().mid(1);
    return suffix;
}

QVariant xcodeInfoPlistOutputEncoding(const GeneratableProject &,
                                      const Project &project,
                                      const GeneratableProductData &,
                                      const ProductData &product,
                                      const GroupData &group)
{
    const QString format = qbsModuleProperty(QStringLiteral("cpp"), QStringLiteral("infoPlistFormat"), project, product, group).toString();
    if (format == QStringLiteral("xml1"))
        return QStringLiteral("XML");
    else if (format == QStringLiteral("binary1"))
        return QStringLiteral("binary");

    // If we don't know what the format is, return the original string (which may be invalid)
    // "same-as-input" is valid
    return format;
}

QVariant xcodeArchitecture(const GeneratableProject &,
                           const Project &project,
                           const GeneratableProductData &,
                           const ProductData &product,
                           const GroupData &group)
{
    return qbsModuleProperty(QStringLiteral("cpp"), QStringLiteral("targetArch"),
                             project, product, group).toString();
}

QVariant xcodePreprocessorDefinitions(const GeneratableProject &,
                                      const qbs::Project &project,
                                      const GeneratableProductData &,
                                      const ProductData &product,
                                      const GroupData &group)
{
    QStringList defs;
    const QVariantList defines = qbsModuleProperties(QStringLiteral("cpp"), QStringLiteral("defines"), project, product, group);
    for (const QVariant &def : defines) {
        // Preprocessor definitions need to be double escaped since Xcode ignores quoted strings
        defs.append(def.toString().replace(QStringLiteral("\""), QStringLiteral("\\\"")));
    }
    return defs;
}

static QStringList toStringList(const QVariantList &list)
{
    QStringList stringList;
    for (const auto &value : list)
        stringList.append(value.toString());
    return stringList;
}

QVariant xcodeSdkRoot(const GeneratableProject &,
                      const Project &project,
                      const GeneratableProductData &,
                      const ProductData &product,
                      const GroupData &group)
{
    const QString sdkName = qbsModuleProperty(QStringLiteral("xcode"), QStringLiteral("sdkName"), project, product, group).toString();
    if (!sdkName.isEmpty())
        return sdkName;

    const QString sysroot = qbsModuleProperty(QStringLiteral("qbs"), QStringLiteral("sysroot"), project, product, group).toString();
    if (!sysroot.isEmpty())
        return sysroot;

    const QVariantList targetOS = qbsModuleProperties(QStringLiteral("qbs"), QStringLiteral("targetOS"), project, product, group);
    if (targetOS.contains(QStringLiteral("macos")))
        return QStringLiteral("macosx");
    else if (targetOS.contains(QStringLiteral("ios")))
        return QStringLiteral("iphoneos");
    else if (targetOS.contains(QStringLiteral("tvos")))
        return QStringLiteral("tvos");
    else if (targetOS.contains(QStringLiteral("watchos")))
        return QStringLiteral("watchos");
    else
        throw ErrorInfo(Tr::tr("Unsupported platform '%1' for Xcode project")
                        .arg(toStringList(targetOS).join(QStringLiteral(", "))));
}

QVariant xcodeSupportedPlatforms(const GeneratableProject &,
                                 const Project &project,
                                 const GeneratableProductData &,
                                 const ProductData &product,
                                 const GroupData &group)
{
    QStringList supportedPlatforms;
    const QVariantList targetOS = qbsModuleProperties(QStringLiteral("qbs"), QStringLiteral("targetOS"), project, product, group);
    if (targetOS.contains(QStringLiteral("macos"))) {
        supportedPlatforms.append(QStringLiteral("macosx"));
    } else if (targetOS.contains(QStringLiteral("ios"))) {
        if (targetOS.contains(QStringLiteral("ios-simulator")))
            supportedPlatforms.append(QStringLiteral("iphonesimulator"));
        supportedPlatforms.append(QStringLiteral("iphoneos"));
    } else if (targetOS.contains(QStringLiteral("tvos"))) {
        if (targetOS.contains(QStringLiteral("tvos-simulator")))
            supportedPlatforms.append(QStringLiteral("appletvsimulator"));
        supportedPlatforms.append(QStringLiteral("appletvos"));
    } else if (targetOS.contains(QStringLiteral("watchos"))) {
        if (targetOS.contains(QStringLiteral("watchos-simulator")))
            supportedPlatforms.append(QStringLiteral("watchsimulator"));
        supportedPlatforms.append(QStringLiteral("watchos"));
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

QVariant _xcodeTargetedDeviceFamily(const Project &project,
                                    const ProductData &product,
                                    const GroupData &group,
                                    const QString &moduleName)
{
    QStringList targetedDeviceFamily;
    const QVariantList targetDevices = qbsModuleProperties(moduleName, QStringLiteral("targetDevices"), project, product, group);
    for (const QVariant &targetDevice : targetDevices) {
        int devId = targetDeviceFamilyIdentifierFromName(targetDevice.toString());
        if (devId != 0)
            targetedDeviceFamily.append(QString::number(devId));
    }
    return targetedDeviceFamily.join(QLatin1Char(','));
}

QVariant xcodeResourcesTargetedDeviceFamily(const GeneratableProject &,
                                            const Project &project,
                                            const GeneratableProductData &,
                                            const ProductData &product,
                                            const GroupData &group)
{
    return _xcodeTargetedDeviceFamily(project, product, group, QStringLiteral("ib"));
}

QVariant xcodeTargetedDeviceFamily(const GeneratableProject &,
                                   const Project &project,
                                   const GeneratableProductData &,
                                   const ProductData &product,
                                   const GroupData &group)
{
    return _xcodeTargetedDeviceFamily(project, product, group, QStringLiteral("xcode"));
}

QVariant xcodeDylibVersion(const GeneratableProject &,
                           const Project &,
                           const GeneratableProductData &,
                           const ProductData &product,
                           const GroupData &)
{
    if (product.type().contains(QStringLiteral("dynamiclibrary")))
        return product.version();
    return QString();
}

QVariant xcodeProductName(const GeneratableProject &,
                          const Project &,
                          const GeneratableProductData &,
                          const ProductData &product,
                          const GroupData &)
{
    if (!product.targetName().isEmpty())
        return product.targetName();
    return product.name();
}

QVariant xcodeUseHeadermap(const GeneratableProject &,
                           const Project &,
                           const GeneratableProductData &,
                           const ProductData &,
                           const GroupData &)
{
    return QVariant(false); // We never want to use headermaps in qbs-generated projects
}

} // namespace qbs

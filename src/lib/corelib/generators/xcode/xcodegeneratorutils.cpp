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

#include "xcodegeneratorutils.h"
#include <tools/qbsassert.h>

namespace qbs {

QString XcodeGeneratorUtils::makePathRelativeTo(const QString &path, const QString &basePath)
{
    QString rootPath(basePath);
    if (!rootPath.endsWith(QLatin1Char('/')))
        rootPath += QLatin1Char('/');

    QString filePath(path);
    if (filePath.startsWith(rootPath))
        filePath.remove(0, rootPath.size());

    return filePath;
}

bool XcodeGeneratorUtils::isXcodeDefaultConfiguration(const QString &qbsConfigurationName,
                                                      QStringList allQbsConfigurations)
{
    static const QString xcodeDefaultConfigurationName = QStringLiteral("Release");
    if (allQbsConfigurations.contains(xcodeDefaultConfigurationName))
        return xcodeConfigurationName(qbsConfigurationName) == xcodeDefaultConfigurationName;
    std::sort(allQbsConfigurations.begin(), allQbsConfigurations.end());
    return qbsConfigurationName == allQbsConfigurations.first();
}

QString XcodeGeneratorUtils::xcodeConfigurationName(QString variantName)
{
    QBS_CHECK(!variantName.isEmpty());
    variantName.replace(0, 1, variantName[0].toUpper());
    return variantName;
}

// Order is significant due to productTypeIdentifier() search path
// See _productTypeIdentifiers in bundle.js
static const QMap<QString, PBX::PBXProductType> productTypeIdentifiers() {
    static QMap<QString, PBX::PBXProductType> map {
        {QStringLiteral("inapppurchase"), PBX::XCInAppPurchaseContentProductType},
        {QStringLiteral("applicationextension"), PBX::XCApplicationExtensionProductType},
        {QStringLiteral("xpcservice"), PBX::XCXPCServiceProductType},
        {QStringLiteral("application"), PBX::PBXApplicationProductType},
        {QStringLiteral("dynamiclibrary"), PBX::PBXFrameworkProductType},
        {QStringLiteral("loadablemodule"), PBX::PBXBundleProductType},
        {QStringLiteral("staticlibrary"), PBX::XCStaticFrameworkProductType},
        {QStringLiteral("kernelmodule"), PBX::XCKernelExtensionProductType},
    };
    return map;
}

/*!
 * Returns the Xcode product type identifier corresponding to the Qbs product type name.
 */
PBX::PBXProductType XcodeGeneratorUtils::xcodeProductType(const GeneratableProductData &product)
{
    const QStringList productType = product.type();
    const bool isBundle = product.uniqueValue<bool>([](const qbs::ProductData &data) {
        return data.moduleProperties().getModuleProperty(QStringLiteral("bundle"),
                                                         QStringLiteral("isBundle")).toBool();
    }, QStringLiteral("bundle.isBundle must have the same values across all configurations "
                      "of product '%1'").arg(product.name()));
    if (isBundle) {
        for (const QString &key : productTypeIdentifiers()) {
            if (productType.contains(key))
                return productTypeIdentifiers()[key];
        }
    } else {
        if (productType.contains(QStringLiteral("application")))
            return PBX::PBXToolProductType;
        if (productType.contains(QStringLiteral("dynamiclibrary")))
            return PBX::PBXDynamicLibraryProductType;
        if (productType.contains(QStringLiteral("staticlibrary")))
            return PBX::PBXStaticLibraryProductType;
    }

    return PBX::PBXProductType();
}

/*!
 * \brief If the value is a path or list of paths,
 * returns it with the prefixes of paths replaced by $(SRCROOT), $(PROJECT_FILE_PATH)/.., etc.,
 * as appropriate.
 * If the value is not a path, returns the original value unmodified.
 */
QVariant XcodeGeneratorUtils::xcodeInsertPlaceholdersInValue(const GeneratableProject &project,
                                                             const QVariant &variant)
{
    if (variant.type() == QVariant::List) {
        QVariantList list;
        for (const QVariant &v : variant.toList())
            list.append(xcodeInsertPlaceholdersInValue(project, v));
        return list;
    }

    // If the value is a path and it begins with the value of SRCROOT or PROJECT_FILE_PATH/.., replace it
    if (variant.type() == QVariant::String) {
        if (variant.toString().startsWith(QDir::homePath()))
            return variant.toString().replace(0, QDir::homePath().size(), QStringLiteral("$(HOME)"));

        // This should end up being the same as PBXProject::projectDirPath(), but if projectDirPath
        // is the empty string (for example, when the xcodeproj resides in the qbs source directory,
        // things would break, so prefer the Qbs value (plus, it saves a parameter to this function)
        const QDir sourceDirectory = project.filePath().absolutePath();
        if (variant.toString().startsWith(sourceDirectory.absolutePath()))
            return variant.toString().replace(0, sourceDirectory.absolutePath().size(),
                                              QStringLiteral("$(SRCROOT)"));

        const QDir buildDirectory = project.baseBuildDirectory();
        Q_ASSERT(buildDirectory.isAbsolute() && buildDirectory.exists());
        if (variant.toString().startsWith(buildDirectory.absolutePath()))
            return variant.toString().replace(0, buildDirectory.absolutePath().size(),
                                              QStringLiteral("$(QBS_BUILD_DIRECTORY)/$(CONFIGURATION)"));
    }

    return variant;
}

} // namespace qbs

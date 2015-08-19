/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing
**
** This file is part of the Qt Build Suite.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms and
** conditions see http://www.qt.io/terms-conditions. For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, The Qt Company gives you certain additional
** rights.  These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

import FileInfo = require("../FileInfo/fileinfo");

export function applicationFileName(product: Product) {
    return product.moduleProperty<string>("cpp", "executablePrefix")
        + product.targetName
        + product.moduleProperty<string>("cpp", "executableSuffix");
}

export function applicationFilePath(product: Product) {
    if (product.moduleProperty<boolean>("bundle", "isBundle"))
        return product.moduleProperty<string>("bundle", "executablePath");
    else
        return applicationFileName(product);
}

export function loadableModuleFileName(product: Product) {
    return product.moduleProperty<string>("cpp", "loadableModulePrefix")
        + product.targetName
        + product.moduleProperty<string>("cpp", "loadableModuleSuffix");
}

export function loadableModuleFilePath(product: Product) {
    if (product.moduleProperty<boolean>("bundle", "isBundle"))
        return product.moduleProperty<string>("bundle", "executablePath");
    else
        return loadableModuleFileName(product);
}

export function staticLibraryFileName(product: Product) {
    return product.moduleProperty<string>("cpp", "staticLibraryPrefix")
        + product.targetName
        + product.moduleProperty<string>("cpp", "staticLibrarySuffix");
}

export function staticLibraryFilePath(product: Product) {
    if (product.moduleProperty<boolean>("bundle", "isBundle"))
        return product.moduleProperty<string>("bundle", "executablePath");
    else
        return staticLibraryFileName(product);
}

export function dynamicLibraryFileName(product: Product, version?: string, maxParts?: number) {
    // If no override version was given, use the product's version
    // We specifically want to differentiate between undefined and i.e.
    // empty string as empty string should be taken to mean "no version"
    // and undefined should be taken to mean "use the product's version"
    // (which could still end up being "no version")
    if (version === undefined)
        version = product.moduleProperty<string>("cpp", "internalVersion");

    // If we have a version number, potentially strip off some components
    if (maxParts === 0)
        version = undefined;
    else if (maxParts && version)
        version = version.split('.').slice(0, maxParts).join('.');

    // Start with prefix + name (i.e. libqbs, qbs)
    var fileName = product.moduleProperty<string>("cpp", "dynamicLibraryPrefix") + product.targetName;

    // For Darwin platforms, append the version number if there is one (i.e. libqbs.1.0.0)
    var targetOS = product.moduleProperties<string>("qbs", "targetOS");
    if (version && targetOS.contains("darwin")) {
        fileName += "." + version;
        version = undefined;
    }

    // Append the suffix (i.e. libqbs.1.0.0.dylib, libqbs.so, qbs.dll)
    fileName += product.moduleProperty<string>("cpp", "dynamicLibrarySuffix");

    // For non-Darwin Unix platforms, append the version number if there is one (i.e. libqbs.so.1.0.0)
    if (version && targetOS.contains("unix") && !targetOS.contains("darwin"))
        fileName += "." + version;

    return fileName;
}

export function dynamicLibraryFilePath(product: Product, version: string, maxParts: number) {
    if (product.moduleProperty("bundle", "isBundle"))
        return product.moduleProperty<string>("bundle", "executablePath");
    else
        return dynamicLibraryFileName(product, version, maxParts);
}

export function importLibraryFilePath(product: Product) {
    return product.moduleProperty<string>("cpp", "dynamicLibraryPrefix")
        + product.targetName
        + product.moduleProperty<string>("cpp", "dynamicLibraryImportSuffix");
}

export function debugInfoIsBundle(product: Product) {
    var flags = product.moduleProperties<string>("cpp", "dsymutilFlags");
    return !flags.contains("-f") && !flags.contains("--flat");
}

export function debugInfoFileName(product: Product) {
    var suffix = "";

    // For bundled dSYMs, the suffix appears on the bundle name, not the actual debug info file
    if (!product.moduleProperties<string>("qbs", "targetOS").contains("darwin")
        || !debugInfoIsBundle(product))
        suffix = product.moduleProperty<string>("cpp", "debugInfoSuffix");

    if (product.moduleProperty<boolean>("bundle", "isBundle")) {
        if (!debugInfoIsBundle(product))
            return product.moduleProperty<string>("bundle", "bundleName") + suffix;
    } else if (product.type.contains("application"))
        return applicationFileName(product) + suffix;
    else if (product.type.contains("dynamiclibrary"))
        return dynamicLibraryFileName(product) + suffix;
    else if (product.type.contains("loadablemodule"))
        return loadableModuleFileName(product) + suffix;
    else if (product.type.contains("staticlibrary"))
        return staticLibraryFileName(product) + suffix;

    return product.targetName + suffix;
}

export function debugInfoBundlePath(product: Product) {
    if (!debugInfoIsBundle(product))
        return undefined;
    var suffix = product.moduleProperty<string>("cpp", "debugInfoSuffix");
    if (product.moduleProperties<string>("qbs", "targetOS").contains("darwin")
        && product.moduleProperty<boolean>("bundle", "isBundle"))
        return product.moduleProperty<string>("bundle", "bundleName") + suffix;
    return debugInfoFileName(product) + suffix;
}

export function debugInfoFilePath(product: Product) {
    var name = debugInfoFileName(product);
    if (product.moduleProperties<string>("qbs", "targetOS").contains("darwin") && debugInfoIsBundle(product)) {
        return FileInfo.joinPaths(debugInfoBundlePath(product), "Contents", "Resources", "DWARF",
            name);
    }

    return name;
}

export function debugInfoPlistFilePath(product: Product) {
    if (!debugInfoIsBundle(product))
        return undefined;
    return FileInfo.joinPaths(debugInfoBundlePath(product), "Contents", "Info.plist");
}

// Returns whether the string looks like a library filename
export function isLibraryFileName(product: Product, fileName: string, prefix: string, suffixes: string[], isShared: boolean) {
    var suffix: string, i: number;
    var os = product.moduleProperties<string>("qbs", "targetOS");
    for (i = 0; i < suffixes.length; ++i) {
        suffix = suffixes[i];
        if (isShared && os.contains("unix") && !os.contains("darwin"))
            suffix += "(\\.[0-9]+){0,3}";
        if (fileName.match("^" + prefix + ".+?\\" + suffix + "$"))
            return true;
    }
    return false;
}

export function frameworkExecutablePath(frameworkPath: string) {
    var suffix = ".framework";
    var isAbsoluteFrameworkPath = frameworkPath.slice(-suffix.length) === suffix;
    if (isAbsoluteFrameworkPath) {
        var frameworkName = FileInfo.fileName(frameworkPath).slice(0, -suffix.length);
        return FileInfo.joinPaths(frameworkPath, frameworkName);
    }
    return undefined;
}

// pathList is also a string, using the respective separator
export function prependOrSetPath(path: string, pathList: string[], separator: string) {
    if (!pathList || pathList.length === 0)
        return path;
    return path + separator + pathList;
}

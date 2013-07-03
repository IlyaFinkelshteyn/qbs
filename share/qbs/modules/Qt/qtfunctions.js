// helper functions for the Qt modules

function getPlatformLibraryName(name, qtcore, qbs, libType)
{
    var libName = name;
    if (qbs.targetOS.contains('windows')) {
        libName += (qbs.enableDebugCode ? 'd' : '');
        if (qtcore.versionMajor < 5)
            libName += qtcore.versionMajor;
        if (!qbs.toolchain.contains("mingw") && libType !== 'dll')
            libName += '.lib';
    }
    if (qbs.targetOS.contains("darwin")) {
        if (!qtcore.frameworkBuild && qtcore.buildVariant.contains("debug") &&
                (!qtcore.buildVariant.contains("release") || qbs.enableDebugCode))
            libName += '_debug';
    }
    return libName;
}

function getQtLibraryName(qtModule, qtcore, qbs, libType)
{
    var libName = "Qt";
    if (qtcore.versionMajor >= 5 && !qtcore.frameworkBuild)
        libName += qtcore.versionMajor;
    libName += qtModule;
    return getPlatformLibraryName(libName, qtcore, qbs, libType);
}

function getQtPluginName(qtPluginName, qtcore, qbs)
{
    return getPlatformLibraryName(qtPluginName, qtcore, qbs, 'dll');
}

function getQtLibraryFilePath(qtModule, qtcore, qbs, cpp)
{
    var path = "";
    if (qbs.targetOS.contains("windows"))
        path += qtcore.binPath;
    else
        path += qtcore.libPath;

    path += "/";

    var libName = getQtLibraryName(qtModule, qtcore, qbs, 'dll');

    if (qtcore.frameworkBuild) {
        path += libName + ".framework";
    } else if (qtcore.staticBuild) {
        path += cpp.staticLibraryPrefix + libName + cpp.staticLibrarySuffix;
    } else {
        path += cpp.dynamicLibraryPrefix + libName;

        if (qbs.targetOS.contains("darwin"))
            path += qtcore.version + "." + cpp.dynamicLibrarySuffix;
        else if (qbs.targetOS.contains("unix"))
            path += cpp.dynamicLibrarySuffix + "." + qtcore.version;
    }

    return path;
}

function getQtPluginFilePath(qtPluginName, qtPluginClass, qtcore, qbs, cpp)
{
    if (qtcore.staticBuild)
        throw "Qt plugins are not available in static builds of Qt";
    var path = qtcore.pluginPath + "/" + qtPluginClass + "/";
    path += cpp.dynamicLibraryPrefix;
    path += getQtPluginName(qtPluginName, qtcore, qbs);
    path += cpp.dynamicLibrarySuffix;
    return path;
}

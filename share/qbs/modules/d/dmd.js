function linkerFlags(product, inputs)
{
    var libraryPaths = ModUtils.moduleProperties(product, 'libraryPaths');
    var dynamicLibraries = ModUtils.moduleProperties(product, "dynamicLibraries");
    var staticLibraries = ModUtils.modulePropertiesFromArtifacts(product, inputs.staticlibrary, 'd', 'staticLibraries');
    var frameworkPaths = ModUtils.moduleProperties(product, 'frameworkPaths');
    var systemFrameworkPaths = ModUtils.moduleProperties(product, 'systemFrameworkPaths');
    var linkerScripts = ModUtils.moduleProperties(product, 'linkerScripts');
    var frameworks = ModUtils.moduleProperties(product, 'frameworks');
    var weakFrameworks = ModUtils.moduleProperties(product, 'weakFrameworks');
    var rpaths = ModUtils.moduleProperties(product, 'rpaths');
    var args = [], i, prefix, suffix, suffixes;

    for (i in rpaths) {
        args.push('-L-rpath');
        args.push('-L' + rpaths[i]);
    }

    // Add filenames of internal library dependencies to the lists
    for (i in inputs.staticlibrary)
        staticLibraries.unshift(inputs.staticlibrary[i].filePath);
    for (i in inputs.dynamiclibrary_copy)
        dynamicLibraries.unshift(inputs.dynamiclibrary_copy[i].filePath);
    for (i in inputs.frameworkbundle)
        frameworks.unshift(inputs.frameworkbundle[i].filePath);

    // Flags for library search paths
    if (libraryPaths)
        args = args.concat(libraryPaths.map(function(path) { return '-L-L' + path }));
    if (frameworkPaths)
        args = args.concat(frameworkPaths.map(function(path) { return '-L-F' + path }));
    if (systemFrameworkPaths)
        args = args.concat(systemFrameworkPaths.map(function(path) { return '-L-iframework' + path }));

    if (linkerScripts)
        args = args.concat(linkerScripts.map(function(path) { return '-L-T' + path }));

    prefix = ModUtils.moduleProperty(product, "staticLibraryPrefix");
    suffixes = ModUtils.moduleProperty(product, "supportedStaticLibrarySuffixes");
    args.push("-L-Bstatic");
    for (i in staticLibraries) {
        if (isLibraryFileName(product, FileInfo.fileName(staticLibraries[i]), prefix, suffixes,
                              false)) {
            args.push('-L' + staticLibraries[i]);
        } else {
            args.push('-L-l' + staticLibraries[i]);
        }
    }
    args.push("-L-Bdynamic")

    prefix = ModUtils.moduleProperty(product, "dynamicLibraryPrefix");
    suffix = ModUtils.moduleProperty(product, "dynamicLibrarySuffix");
    for (i in dynamicLibraries) {
        if (isLibraryFileName(product, FileInfo.fileName(dynamicLibraries[i]), prefix, [suffix],
                              true)) {
            args.push(dynamicLibraries[i]);
        } else {
            args.push('-L-l' + dynamicLibraries[i]);
        }
    }

    suffix = ".framework";
    for (i in frameworks) {
        if (frameworks[i].slice(-suffix.length) === suffix)
            args.push(frameworks[i] + "/" + FileInfo.fileName(frameworks[i]).slice(0, -suffix.length));
        else
            args = args.concat(['-framework', frameworks[i]]);
    }

    for (i in weakFrameworks) {
        if (weakFrameworks[i].slice(-suffix.length) === suffix)
            args = args.concat(['-L-weak_library', weakFrameworks[i] + "/" + FileInfo.fileName(weakFrameworks[i]).slice(0, -suffix.length)]);
        else
            args = args.concat(['-L-weak_framework', weakFrameworks[i]]);
    }

    var isDarwin = product.moduleProperty("qbs", "targetOS").contains("darwin");
    var unresolvedSymbolsAction = isDarwin ? "error" : "ignore-in-shared-libs";
    if (ModUtils.moduleProperty(product, "allowUnresolvedSymbols"))
        unresolvedSymbolsAction = isDarwin ? "suppress" : "ignore-all";
    var unresolvedSymbolsKey = isDarwin ? "-undefined," : "--unresolved-symbols=";
    args.push("-L" + unresolvedSymbolsKey + unresolvedSymbolsAction);

    if (product.moduleProperty("qbs", "targetOS").contains('linux')) {
        var transitiveSOs = ModUtils.modulePropertiesFromArtifacts(product,
                                                                   inputs.dynamiclibrary_copy, 'd', 'transitiveSOs')
        var uniqueSOs = [].uniqueConcat(transitiveSOs)
        for (i in uniqueSOs) {
            // The real library is located one level up.
            args.push("-L-rpath-link=" + FileInfo.path(FileInfo.path(uniqueSOs[i])));
        }
    }

    return args;
}

// Returns whether the string looks like a library filename
function isLibraryFileName(product, fileName, prefix, suffixes, isShared)
{
    var suffix, i;
    var os = product.moduleProperty("qbs", "targetOS");
    for (i = 0; i < suffixes.length; ++i) {
        suffix = suffixes[i];
        if (isShared && os.contains("unix") && !os.contains("darwin"))
            suffix += "(\\.[0-9]+){0,3}";
        if (fileName.match("^" + prefix + ".+?\\" + suffix + "$"))
            return true;
    }
    return false;
}

// for compiler AND linker
function configFlags(config) {
    var args = [];

    var arch = ModUtils.moduleProperty(config, "architecture")
    if (arch === 'x86_64')
        args.push('-m64');
    else if (arch === 'x86')
        args.push('-m32');

    if (ModUtils.moduleProperty(config, "debugInformation")) {
        args.push('-g');
        args.push('-debug');
    }
    var opt = ModUtils.moduleProperty(config, "optimization")
    if (opt === 'fast')
        args.push('-O');

    var warnings = ModUtils.moduleProperty(config, "warningLevel")
    if (warnings === 'all')
        args.push('-w');

    if (!ModUtils.moduleProperty(config, "treatWarningsAsErrors"))
        args.push('-wi');
    return args;
}

function removePrefixAndSuffix(str, prefix, suffix)
{
    return str.substr(prefix.length, str.length - prefix.length - suffix.length);
}

// ### what we actually need here is something like product.usedFileTags
//     that contains all fileTags that have been used when applying the rules.
function additionalCompilerFlags(product, input, output)
{
    var importPaths = ModUtils.moduleProperties(input, 'importPaths');

    var EffectiveTypeEnum = { UNKNOWN: 0, LIB: 1, APP: 2 };
    var effectiveType = EffectiveTypeEnum.UNKNOWN;
    var libTypes = {staticlibrary : 1, dynamiclibrary : 1, frameworkbundle : 1};
    var appTypes = {application : 1, applicationbundle : 1};
    var i;
    for (i = product.type.length; --i >= 0;) {
        if (libTypes.hasOwnProperty(product.type[i]) !== -1) {
            effectiveType = EffectiveTypeEnum.LIB;
            break;
        } else if (appTypes.hasOwnProperty(product.type[i]) !== -1) {
            effectiveType = EffectiveTypeEnum.APP;
            break;
        }
    }

    var args = []
    if (product.moduleProperty('d', 'positionIndependentCode'))
        args.push('-fPIC');

    for (i in importPaths)
        args.push('-I' + importPaths[i]);

    args.push('-c');
    args.push(input.filePath);
    args.push('-of' + output.filePath);
    return args
}

function additionalCompilerAndLinkerFlags(product) {
    var args = []

    return args
}

function majorVersion(version, defaultValue)
{
    var n = parseInt(product.version, 10);
    return isNaN(n) ? defaultValue : n;
}

function soname(product, outputFilePath)
{
    var outputFileName = FileInfo.fileName(outputFilePath);
    if (product.version) {
        var major = majorVersion(product.version);
        if (major) {
            return outputFileName.substr(0, outputFileName.length - product.version.length)
                    + major;
        }
    }
    return outputFileName;
}

function prepareCompiler(project, product, inputs, outputs, input, output)
{
    var args = configFlags(input);

    args = args.concat(ModUtils.moduleProperties(input, 'platformCommonCompilerFlags'));

    args = args.concat(ModUtils.moduleProperties(input, 'platformFlags', 'd'),
                       ModUtils.moduleProperties(input, 'flags', 'd'));

    args = args.concat(ModUtils.moduleProperties(input, 'commonCompilerFlags'));
    args = args.concat(additionalCompilerFlags(product, input, output));
    args = args.concat(additionalCompilerAndLinkerFlags(product));

    var compilerPath = ModUtils.moduleProperty(product, "compilerPath");
    var wrapperArgs = ModUtils.moduleProperty(product, "compilerWrapper");
    if (wrapperArgs && wrapperArgs.length > 0) {
        args.unshift(compilerPath);
        compilerPath = wrapperArgs.shift();
        args = wrapperArgs.concat(args);
    }

    var cmd = new Command(compilerPath, args);
    cmd.description = 'compiling ' + FileInfo.fileName(input.filePath);
    cmd.highlight = "compiler";
    cmd.responseFileUsagePrefix = '@';
    return cmd;
}

// Concatenates two arrays of library names and preserves the dependency order that ld needs.
function concatLibs(libs, deplibs)
{
    var r = [];
    var s = {};

    function addLibs(lst)
    {
        for (var i = lst.length; --i >= 0;) {
            var lib = lst[i];
            if (!s[lib]) {
                s[lib] = true;
                r.unshift(lib);
            }
        }
    }

    addLibs(deplibs);
    addLibs(libs);
    return r;
}

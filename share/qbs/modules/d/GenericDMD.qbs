import qbs 1.0
import qbs.File
import qbs.FileInfo
import qbs.ModUtils
import qbs.Process
import 'dmd.js' as Dmd
import 'path-tools.js' as PathTools

DModule {
    condition: false

    property stringList transitiveSOs
    property string toolchainPrefix
    property path toolchainInstallPath
    compilerName: 'dmd'
    linkerName: compilerName
    property string archiverName: 'ar'
    property string nmName: 'nm'
    property path sysroot: qbs.sysroot
    property path platformPath

    property string toolchainPathPrefix: {
        var path = ''
        if (toolchainInstallPath) {
            path += toolchainInstallPath
            if (path.substr(-1) !== '/')
                path += '/'
        }
        if (toolchainPrefix)
            path += toolchainPrefix
        return path
    }

    compilerPath: toolchainPathPrefix + compilerName
    linkerPath: toolchainPathPrefix + linkerName
    property path archiverPath: { return toolchainPathPrefix + archiverName }
    property path nmPath: { return toolchainPathPrefix + nmName }

    readonly property bool shouldCreateSymlinks: {
        return createSymlinks && product.version &&
                !product.type.contains("frameworkbundle") && qbs.targetOS.contains("unix");
    }

    readonly property string internalVersion: {
        if (product.version === undefined)
            return undefined;

        if (typeof product.version !== "string"
                || !product.version.match(/^([0-9]+\.){0,3}[0-9]+$/))
            throw("product.version must be a string in the format x[.y[.z[.w]] "
                + "where each component is an integer");

        var maxVersionParts = 3;
        var versionParts = product.version.split('.').slice(0, maxVersionParts);

        // pad if necessary
        for (var i = versionParts.length; i < maxVersionParts; ++i)
            versionParts.push("0");

        return versionParts.join('.');
    }

    Rule {
        id: dynamicLibraryLinker
        multiplex: true
        inputs: ["obj"]
        usings: ["dynamiclibrary_copy", "staticlibrary", "frameworkbundle"]

        Artifact {
            fileName: product.destinationDirectory + "/" + PathTools.dynamicLibraryFilePath()
            fileTags: ["dynamiclibrary"]
        }

        // libfoo
        Artifact {
            fileName: product.destinationDirectory + "/" + PathTools.dynamicLibraryFileName(undefined, 0)
            fileTags: ["dynamiclibrary_symlink"]
        }

        // libfoo.1
        Artifact {
            fileName: product.destinationDirectory + "/" + PathTools.dynamicLibraryFileName(undefined, 1)
            fileTags: ["dynamiclibrary_symlink"]
        }

        // libfoo.1.0
        Artifact {
            fileName: product.destinationDirectory + "/" + PathTools.dynamicLibraryFileName(undefined, 2)
            fileTags: ["dynamiclibrary_symlink"]
        }

        // Copy of dynamic lib for smart re-linking.
        Artifact {
            fileName: product.destinationDirectory + "/.socopy/"
                      + PathTools.dynamicLibraryFilePath()
            fileTags: ["dynamiclibrary_copy"]
            alwaysUpdated: false
            d.transitiveSOs: {
                var result = []
                for (var i in inputs.dynamiclibrary_copy) {
                    var lib = inputs.dynamiclibrary_copy[i]
                    var impliedLibs = ModUtils.moduleProperties(lib, 'transitiveSOs')
                    var libsToAdd = [lib.filePath].concat(impliedLibs);
                    result = result.concat(libsToAdd);
                }
                result = Dmd.concatLibs([], result);
                return result;
            }
        }

        prepare: {
            // Actual linker command.
            var libFilePath = outputs["dynamiclibrary"][0].filePath;
            var platformLinkerFlags = ModUtils.moduleProperties(product, 'platformLinkerFlags');
            var linkerFlags = ModUtils.moduleProperties(product, 'linkerFlags');
            var commands = [];
            var i;
            var args = Dmd.configFlags(product);
            args.push('-shared');
            if (product.moduleProperty("qbs", "targetOS").contains('linux')) {
                args = args.concat([
                    '-L--hash-style=gnu',
                    '-L--as-needed',
                    '-L-soname=' + Dmd.soname(product, libFilePath)
                ]);
            } else if (product.moduleProperty("qbs", "targetOS").contains('darwin')) {
                var installNamePrefix = product.moduleProperty("d", "installNamePrefix");
                if (installNamePrefix !== undefined)
                    args.push("--L-install_name -L"
                              + installNamePrefix + FileInfo.fileName(libFilePath));
                args.push("-L-headerpad_max_install_names");
            }
            args = args.concat(platformLinkerFlags);
            for (i in linkerFlags)
                args.push(linkerFlags[i])
            for (i in inputs.obj)
                args.push(inputs.obj[i].filePath);

            args.push('-o');
            args.push(libFilePath);
            args = args.concat(Dmd.linkerFlags(product, inputs));
            args = args.concat(Dmd.additionalCompilerAndLinkerFlags(product));
            var cmd = new Command(ModUtils.moduleProperty(product, "linkerPath"), args);
            cmd.description = 'linking ' + FileInfo.fileName(libFilePath);
            cmd.highlight = 'linker';
            cmd.responseFileUsagePrefix = '@';
            commands.push(cmd);

            // Update the copy, if any global symbols have changed.
            cmd = new JavaScriptCommand();
            cmd.silent = true;
            cmd.sourceCode = function() {
                var sourceFilePath = outputs.dynamiclibrary[0].filePath;
                var targetFilePath = outputs.dynamiclibrary_copy[0].filePath;
                if (!File.exists(targetFilePath)) {
                    File.copy(sourceFilePath, targetFilePath);
                    return;
                }
                var process = new Process();
                var command = ModUtils.moduleProperty(product, "nmPath");
                var args = ["-g", "-D", "-P"];
                if (process.exec(command, args.concat(sourceFilePath), false) != 0) {
                    // Failure to run the nm tool is not fatal. We just fall back to the
                    // "always relink" behavior.
                    File.copy(sourceFilePath, targetFilePath);
                    return;
                }
                var globalSymbolsSource = process.readStdOut();
                if (process.exec(command, args.concat(targetFilePath), false) != 0) {
                    File.copy(sourceFilePath, targetFilePath);
                    return;
                }
                var globalSymbolsTarget = process.readStdOut();
                process.close();

                var globalSymbolsSourceLines = globalSymbolsSource.split('\n');
                var globalSymbolsTargetLines = globalSymbolsTarget.split('\n');
                if (globalSymbolsSourceLines.length !== globalSymbolsTargetLines.length) {
                    File.copy(sourceFilePath, targetFilePath);
                    return;
                }
                while (globalSymbolsSourceLines.length > 0) {
                    var sourceLine = globalSymbolsSourceLines.shift();
                    var targetLine = globalSymbolsTargetLines.shift();
                    var sourceLineElems = sourceLine.split(/\s+/);
                    var targetLineElems = targetLine.split(/\s+/);
                    if (sourceLineElems[0] !== targetLineElems[0] // Object name.
                            || sourceLineElems[1] !== targetLineElems[1]) { // Object type
                        File.copy(sourceFilePath, targetFilePath);
                        return;
                    }
                }
            }
            commands.push(cmd);

            // Create symlinks from {libfoo, libfoo.1, libfoo.1.0} to libfoo.1.0.0
            if (ModUtils.moduleProperty(product, "shouldCreateSymlinks")) {
                var links = outputs["dynamiclibrary_symlink"];
                var symlinkCount = links.length;
                for (var i = 0; i < symlinkCount; ++i) {
                    cmd = new Command("ln", ["-sf", FileInfo.fileName(libFilePath),
                                                    links[i].filePath]);
                    cmd.highlight = "filegen";
                    cmd.description = "creating symbolic link '"
                            + FileInfo.fileName(links[i].filePath) + "'";
                    cmd.workingDirectory = FileInfo.path(libFilePath);
                    commands.push(cmd);
                }
            }
            return commands;
        }
    }

    Rule {
        id: staticLibraryLinker
        multiplex: true
        inputs: ["obj"]
        usings: ["dynamiclibrary", "staticlibrary", "frameworkbundle"]

        Artifact {
            fileName: product.destinationDirectory + "/" + PathTools.staticLibraryFilePath()
            fileTags: ["staticlibrary"]
            dmd.staticLibraries: {
                var result = []
                for (var i in inputs.staticlibrary) {
                    var lib = inputs.staticlibrary[i]
                    result = Dmd.concatLibs(result, [lib.filePath,
                                                     ModUtils.moduleProperties(lib,
                                                                               'staticLibraries')]);
                }
                return result
            }
            dmd.dynamicLibraries: {
                var result = []
                for (var i in inputs.dynamiclibrary)
                    result.push(inputs.dynamiclibrary[i].filePath);
                return result
            }
        }

        prepare: {
            var args = ['rcs', output.filePath];
            for (var i in inputs.obj)
                args.push(inputs.obj[i].filePath);
            var cmd = new Command(ModUtils.moduleProperty(product, "archiverPath"), args);
            cmd.description = 'creating ' + FileInfo.fileName(output.filePath);
            cmd.highlight = 'linker'
            cmd.responseFileUsagePrefix = '@';
            return cmd;
        }
    }

    Rule {
        id: applicationLinker
        multiplex: true
        inputs: {
            var tags = ["obj"];
            if (product.type.contains("application") &&
                product.moduleProperty("qbs", "targetOS").contains("darwin") &&
                product.moduleProperty("d", "embedInfoPlist"))
                tags.push("infoplist");
            return tags;
        }
        usings: ["dynamiclibrary_copy", "staticlibrary", "frameworkbundle"]

        Artifact {
            fileName: product.destinationDirectory + "/" + PathTools.applicationFilePath()
            fileTags: ["application"]
        }

        prepare: {
            var platformLinkerFlags = ModUtils.moduleProperties(product, 'platformLinkerFlags');
            var linkerFlags = ModUtils.moduleProperties(product, 'linkerFlags');
            var args = Dmd.configFlags(product);
            for (var i in inputs.obj)
                args.push(inputs.obj[i].filePath)
            args = args.concat(platformLinkerFlags);
            for (i in linkerFlags)
                args.push(linkerFlags[i])
            args.push('-of' + output.filePath);

            args = args.concat(Dmd.linkerFlags(product, inputs));
            args = args.concat(Dmd.additionalCompilerAndLinkerFlags(product));

            var cmd = new Command(ModUtils.moduleProperty(product, "linkerPath"), args);
            cmd.description = 'linking ' + FileInfo.fileName(output.filePath);
            cmd.highlight = 'linker'
            cmd.responseFileUsagePrefix = '@';
            return cmd;
        }
    }

    Rule {
        id: compiler
        inputs: ["d"]
        auxiliaryInputs: ["di"]

        Artifact {
            fileTags: ["obj"]
            fileName: ".obj/" + product.name + "/" + input.baseDir + "/" + input.fileName + ".o"
        }

        prepare: {
            return Dmd.prepareCompiler.apply(this, arguments);
        }
    }
}

import qbs 1.0
import qbs.FileInfo
import qbs.ModUtils
import qbs.TextFile
import qbs.Utilities
import "moc.js" as Moc
import "qdoc.js" as Qdoc

Module {
    id: qtcore

    Depends { name: "cpp" }

    property string libInfix: ""
    property stringList config
    property stringList qtConfig
    property path binPath
    property path incPath
    property path libPath
    property path pluginPath
    property path mkspecPath
    property string mocName: "moc"
    property stringList mocFlags: []
    property string lreleaseName: "lrelease"
    property string qdocName: versionMajor >= 5 ? "qdoc" : "qdoc3"
    property stringList qdocEnvironment
    property path docPath
    property stringList helpGeneratorArgs: versionMajor >= 5 ? ["-platform", "minimal"] : []
    property var versionParts: version ? version.split('.').map(function(item) { return parseInt(item, 10); }) : []
    property int versionMajor: versionParts[0]
    property int versionMinor: versionParts[1]
    property int versionPatch: versionParts[2]
    property bool frameworkBuild
    property bool staticBuild
    property stringList pluginMetaData: []

    property stringList availableBuildVariants
    property string qtBuildVariant: {
        if (availableBuildVariants.contains(qbs.buildVariant))
            return qbs.buildVariant;
        return availableBuildVariants.length > 0 ? availableBuildVariants[0] : "";
    }

    property stringList staticLibsDebug: @staticLibsDebug@
    property stringList staticLibsRelease: @staticLibsRelease@
    property stringList dynamicLibsDebug: @dynamicLibsDebug@
    property stringList dynamicLibsRelease: @dynamicLibsRelease@
    property stringList staticLibs: qtBuildVariant === "debug"
                                    ? staticLibsDebug : staticLibsRelease
    property stringList dynamicLibs: qtBuildVariant === "debug"
                                    ? dynamicLibsDebug : dynamicLibsRelease
    property stringList linkerFlagsDebug: @linkerFlagsDebug@
    property stringList linkerFlagsRelease: @linkerFlagsRelease@
    property stringList coreLinkerFlags: qtBuildVariant === "debug"
                                    ? linkerFlagsDebug : linkerFlagsRelease
    property stringList frameworksDebug: @frameworksDebug@
    property stringList frameworksRelease: @frameworksRelease@
    property stringList coreFrameworks: qtBuildVariant === "debug"
            ? frameworksDebug : frameworksRelease
    property stringList frameworkPathsDebug: @frameworkPathsDebug@
    property stringList frameworkPathsRelease: @frameworkPathsRelease@
    property stringList coreFrameworkPaths: qtBuildVariant === "debug"
            ? frameworkPathsDebug : frameworkPathsRelease
    property string libNameForLinkerDebug: @libNameForLinkerDebug@
    property string libNameForLinkerRelease: @libNameForLinkerRelease@
    property string libNameForLinker: qtBuildVariant === "debug"
                                      ? libNameForLinkerDebug : libNameForLinkerRelease
    property string libFilePathDebug: @libFilePathDebug@
    property string libFilePathRelease: @libFilePathRelease@
    property string libFilePath: qtBuildVariant === "debug"
                                      ? libFilePathDebug : libFilePathRelease

    coreLibPaths: @libraryPaths@

    // These are deliberately not path types
    // We don't want to resolve them against the source directory
    property string generatedHeadersDir: product.buildDirectory + "/qt.headers"
    property string generatedFilesDir: generatedHeadersDir // TODO: Remove in 1.8
    property string qdocOutputDir: product.buildDirectory + "/qdoc_html"
    property string qmDir: product.destinationDirectory
    property string qmBaseName: product.targetName
    property bool lreleaseMultiplexMode: false

    cpp.entryPoint: qbs.targetOS.containsAny(["ios", "tvos"])
                        && Utilities.versionCompare(version, "5.6.0") >= 0
                    ? "_qt_main_wrapper"
                    : undefined
    cpp.cxxLanguageVersion: Utilities.versionCompare(version, "5.7.0") >= 0 ? "c++11" : original
    cpp.defines: {
        var defines = @defines@;
        // ### QT_NO_DEBUG must be added if the current build variant is derived
        //     from the build variant "release"
        if (!qbs.debugInformation)
            defines.push("QT_NO_DEBUG");
        if (qbs.targetOS.containsAny(["ios", "tvos"])) {
            defines = defines.concat(["DARWIN_NO_CARBON", "QT_NO_CORESERVICES", "QT_NO_PRINTER",
                            "QT_NO_PRINTDIALOG"]);
            if (Utilities.versionCompare(version, "5.6.0") < 0)
                defines.push("main=qtmn");
        }
        return defines;
    }
    cpp.includePaths: {
        var paths = @includes@;
        paths.push(mkspecPath, generatedHeadersDir);
        return paths;
    }
    cpp.libraryPaths: {
        var libPaths = [libPath];
        if (staticBuild && pluginPath)
            libPaths.push(pluginPath + "/platforms");
        libPaths = libPaths.concat(coreLibPaths);
        return libPaths;
    }
    cpp.staticLibraries: {
        var libs = [];
        if (staticBuild)
            libs.push(libFilePath);
        if (qbs.targetOS.contains('windows') && !product.consoleApplication) {
            libs = libs.concat(qtBuildVariant === "debug"
                               ? @entryPointLibsDebug@ : @entryPointLibsRelease@);
        }
        libs = libs.concat(staticLibs);
        return libs;
    }
    cpp.dynamicLibraries: {
        var libs = [];
        if (!staticBuild && !frameworkBuild)
            libs.push(libFilePath);
        libs = libs.concat(dynamicLibs);
        return libs;
    }
    cpp.linkerFlags: coreLinkerFlags
    cpp.frameworkPaths: coreFrameworkPaths.concat(frameworkBuild ? [libPath] : [])
    cpp.frameworks: {
        var frameworks = coreFrameworks
        if (frameworkBuild)
            frameworks.push(libNameForLinker);
        if (qbs.targetOS.contains('ios') && staticBuild)
            frameworks = frameworks.concat(["Foundation", "CoreFoundation"]);
        if (frameworks.length === 0)
            return undefined;
        return frameworks;
    }
    cpp.rpaths: qbs.targetOS.contains('linux') ? [libPath] : undefined
    cpp.positionIndependentCode: versionMajor >= 5 ? true : undefined
    cpp.cxxFlags: {
        var flags = [];
        if (qbs.toolchain.contains('msvc')) {
            flags.push('/Zm200');
            if (versionMajor < 5)
                flags.push('/Zc:wchar_t-');
        }

        return flags;
    }
    cpp.cxxStandardLibrary: {
        if (qbs.targetOS.contains('darwin') && qbs.toolchain.contains('clang')
                && config.contains('c++11'))
            return "libc++";
        return original;
    }

    additionalProductTypes: ["qm"]

    validate: {
        var validator = new ModUtils.PropertyValidator("Qt.core");
        validator.setRequiredProperty("binPath", binPath);
        validator.setRequiredProperty("incPath", incPath);
        validator.setRequiredProperty("libPath", libPath);
        validator.setRequiredProperty("mkspecPath", mkspecPath);
        validator.setRequiredProperty("version", version);
        validator.setRequiredProperty("config", config);
        validator.setRequiredProperty("qtConfig", qtConfig);
        validator.setRequiredProperty("versionMajor", versionMajor);
        validator.setRequiredProperty("versionMinor", versionMinor);
        validator.setRequiredProperty("versionPatch", versionPatch);

        if (!staticBuild)
            validator.setRequiredProperty("pluginPath", pluginPath);

        // Allow custom version suffix since some distributions might want to do this,
        // but otherwise the version must start with a valid 3-component string
        validator.addVersionValidator("version", version, 3, 3, true);
        validator.addRangeValidator("versionMajor", versionMajor, 1);
        validator.addRangeValidator("versionMinor", versionMinor, 0);
        validator.addRangeValidator("versionPatch", versionPatch, 0);

        validator.addCustomValidator("availableBuildVariants", availableBuildVariants, function (v) {
            return v.length > 0;
        }, "the Qt installation supports no build variants");

        validator.addCustomValidator("qtBuildVariant", qtBuildVariant, function (variant) {
            return availableBuildVariants.contains(variant);
        }, "'" + qtBuildVariant + "' is not supported by this Qt installation");

        validator.addCustomValidator("qtBuildVariant", qtBuildVariant, function (variant) {
            return variant === qbs.buildVariant || !qbs.toolchain.contains("msvc");
        }, " is '" + qtBuildVariant + "', but qbs.buildVariant is '" + qbs.buildVariant
            + "', which is not allowed when using MSVC");

        validator.addFileNameValidator("resourceFileBaseName", resourceFileBaseName);

        validator.validate();
    }

    setupRunEnvironment: {
        var env;
        if (qbs.targetOS.contains('windows')) {
            env = new ModUtils.EnvironmentVariable("PATH", qbs.pathListSeparator, true);
            env.append(binPath);
            env.set();
        } else if (qbs.targetOS.contains("darwin")) {
            env = new ModUtils.EnvironmentVariable("DYLD_FRAMEWORK_PATH", qbs.pathListSeparator);
            env.append(libPath);
            env.set();

            env = new ModUtils.EnvironmentVariable("DYLD_LIBRARY_PATH", qbs.pathListSeparator);
            env.append(libPath);
            env.set();
        }
    }

    FileTagger {
        patterns: ["*.qrc"]
        fileTags: ["qrc"]
    }

    FileTagger {
        patterns: ["*.ts"]
        fileTags: ["ts"]
    }

    FileTagger {
        patterns: ["*.qdoc"]
        fileTags: ["qdoc"]
    }

    FileTagger {
        patterns: ["*.qdocconf"]
        fileTags: ["qdocconf"]
    }

    FileTagger {
        patterns: ["*.qhp"]
        fileTags: ["qhp"]
    }

    Rule {
        name: "QtCoreMocRule"
        inputs: ["objcpp", "cpp", "hpp"]
        auxiliaryInputs: ["qt_plugin_metadata"]
        excludedAuxiliaryInputs: ["unmocable"]
        outputFileTags: ["hpp", "cpp", "unmocable"]
        outputArtifacts: {
            var mocinfo = QtMocScanner.apply(input);
            if (!mocinfo.hasQObjectMacro)
                return [];
            var artifact = { fileTags: ["unmocable"] };
            if (input.fileTags.contains("hpp")) {
                artifact.filePath = ModUtils.moduleProperty(product, "generatedHeadersDir")
                        + "/moc_" + input.completeBaseName + ".cpp";
            } else {
                artifact.filePath = ModUtils.moduleProperty(product, "generatedHeadersDir")
                          + '/' + input.completeBaseName + ".moc";
            }
            artifact.fileTags.push(mocinfo.mustCompile ? "cpp" : "hpp");
            if (mocinfo.hasPluginMetaDataMacro)
                artifact.explicitlyDependsOn = ["qt_plugin_metadata"];
            return [artifact];
        }
        prepare: {
            var cmd = new Command(Moc.fullPath(product),
                                  Moc.args(product, input, output.filePath));
            cmd.description = 'moc ' + input.fileName;
            cmd.highlight = 'codegen';
            return cmd;
        }
    }

    property path resourceSourceBase
    property string resourcePrefix: "/"
    property string resourceFileBaseName: product.targetName
    Rule {
        multiplex: true
        inputs: ["qt.core.resource_data"]
        Artifact {
            filePath: product.moduleProperty("Qt.core", "resourceFileBaseName") + ".qrc"
            fileTags: ["qrc"]
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "generating " + output.fileName;
            cmd.sourceCode = function() {
                var qrcFile = new TextFile(output.filePath, TextFile.WriteOnly);
                try {
                    qrcFile.writeLine('<!DOCTYPE RCC>');
                    qrcFile.writeLine('<RCC version="1.0">');

                    var inputsByPrefix = {}
                    for (var i = 0; i < inputs["qt.core.resource_data"].length; ++i) {
                        var inp = inputs["qt.core.resource_data"][i];
                        var prefix = inp.moduleProperty("Qt.core", "resourcePrefix");
                        var inputsList = inputsByPrefix[prefix] || [];
                        inputsList.push(inp);
                        inputsByPrefix[prefix] = inputsList;
                    }

                    for (var prefix in inputsByPrefix) {
                        qrcFile.writeLine('<qresource prefix="' + prefix + '">');
                        for (var i = 0; i < inputsByPrefix[prefix].length; ++i) {
                            var inp = inputsByPrefix[prefix][i];
                            var fullResPath = inp.filePath;
                            var baseDir = inp.moduleProperty("Qt.core", "resourceSourceBase");
                            var resAlias = baseDir
                                ? FileInfo.relativePath(baseDir, fullResPath) : inp.fileName;
                            qrcFile.writeLine('<file alias="' + resAlias + '">'
                                              + fullResPath + '</file>');
                        }
                        qrcFile.writeLine('</qresource>');
                    }

                    qrcFile.writeLine('</RCC>');
                } finally {
                    qrcFile.close();
                }
            };
            return [cmd];
        }
    }

    Rule {
        inputs: ["qrc"]

        Artifact {
            filePath: "qrc_" + input.completeBaseName + ".cpp"
            fileTags: ["cpp"]
        }
        prepare: {
            var cmd = new Command(ModUtils.moduleProperty(product, "binPath") + '/rcc',
                                  [input.filePath, '-name',
                                   FileInfo.completeBaseName(input.filePath),
                                   '-o', output.filePath]);
            cmd.description = 'rcc ' + input.fileName;
            cmd.highlight = 'codegen';
            return cmd;
        }
    }

    Rule {
        inputs: ["ts"]
        multiplex: lreleaseMultiplexMode

        Artifact {
            filePath: FileInfo.joinPaths(ModUtils.moduleProperty(product, "qmDir"),
                    (ModUtils.moduleProperty(product, "lreleaseMultiplexMode")
                     ? ModUtils.moduleProperty(product, "qmBaseName")
                     : input.baseName) + ".qm")
            fileTags: ["qm"]
        }

        prepare: {
            var inputFilePaths;
            if (ModUtils.moduleProperty(product, "lreleaseMultiplexMode"))
                inputFilePaths = inputs["ts"].map(function(artifact) { return artifact.filePath; });
            else
                inputFilePaths = [input.filePath];
            var args = ['-silent', '-qm', output.filePath].concat(inputFilePaths);
            var cmd = new Command(ModUtils.moduleProperty(product, "binPath") + '/'
                                  + ModUtils.moduleProperty(product, "lreleaseName"), args);
            cmd.description = 'Creating ' + output.fileName;
            cmd.highlight = 'filegen';
            return cmd;
        }
    }

    Rule {
        inputs: "qdocconf-main"
        explicitlyDependsOn: ["qdoc", "qdocconf"]

        outputFileTags: ModUtils.allFileTags(Qdoc.qdocFileTaggers())
        outputArtifacts: Qdoc.outputArtifacts(product, input)

        prepare: {
            var outputDir = ModUtils.moduleProperty(product, "qdocOutputDir");
            var args = Qdoc.qdocArgs(product, input, outputDir);
            var cmd = new Command(ModUtils.moduleProperty(product, "binPath") + '/'
                                  + ModUtils.moduleProperty(product, "qdocName"), args);
            cmd.description = 'qdoc ' + input.fileName;
            cmd.highlight = 'filegen';
            cmd.environment = ModUtils.moduleProperty(product, "qdocEnvironment");
            cmd.environment.push("OUTDIR=" + outputDir); // Qt 4 replacement for -outputdir
            return cmd;
        }
    }

    Rule {
        inputs: "qhp"

        Artifact {
            filePath: input.completeBaseName + ".qch"
            fileTags: ["qch"]
        }

        prepare: {
            var args = [input.filePath];
            args = args.concat(ModUtils.moduleProperty(product, "helpGeneratorArgs"));
            args.push("-o");
            args.push(output.filePath);
            var cmd = new Command(ModUtils.moduleProperty(product, "binPath") + "/qhelpgenerator",
                                  args);
            cmd.description = 'qhelpgenerator ' + input.fileName;
            cmd.highlight = 'filegen';
            cmd.stdoutFilterFunction = function(output) {
                return "";
            };
            return cmd;
        }
    }
}

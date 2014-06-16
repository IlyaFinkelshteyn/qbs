import qbs
import qbs.ModUtils
import qbs.TextFile
import qbs.DarwinTools // for expandPlistEnvironmentVariables, which needs renaming...

Module {
    // {"MY_APP_VERSION":"1.0.0"} means #define MY_APP_VERSION @MY_APP_VERSION@ becomes #define MY_APP_VERSION 1.0.0
    property var replacements

    // allows for custom syntaxes by finding indexOf("var") and then using the
    // characters before and after that substring as the delimiters.
    // fatal error if this syntax is not satisfied (i.e. var not found or 0 chars before or after var)
    //
    // "define" could be a special syntax which replaces #define FOO with #define FOO value if the replacements dict contains FOO,
    // or // #undef FOO if it does not
    // not 100% sure on that one yet but it could certainly be useful...
    property stringList replacementFormats: ["${var}", "$(var)", "@var@", "define"]

    // warning undefined variable XXX in variable expansion
    property bool warnings

    Rule {
        inputs: ["infile"]

        Artifact {
            filePath: input.fileName.endsWith(".in") ? input.fileName.slice(0, -3) : input.fileName
            fileTags: [] // hmm... can we somehow detect from the file extension, i.e. file.h.in > file.h
        }

        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "generating " + output.fileName;
            cmd.highlight = "codegen";
            cmd.sourceCode = function() {
                var inFile;
                var content;
                try {
                    inFile = new TextFile(input.filePath);
                    content = inFile.readAll();
                } finally {
                    inFile.close();
                }

                ModUtils.moduleProperty(input, "replacementFormats") // pass to expandPlistEnvironmentVariables

                // yeah, the API needs to be changed...
                content = expandPlistEnvironmentVariables({"content": content},
                                                          ModUtils.moduleProperty(input, "replacements"),
                                                          ModUtils.moduleProperty(input, "warnings"));
                content = content["content"];

                var outFile;
                try {
                    outFile = new TextFile(output.filePath, TextFile.WriteOnly);
                    outFile.truncate();
                    outFile.write(content);
                } finally {
                    outFile.close();
                }
            }
            return cmd;
        }
    }

    FileTagger {
        patterns: ["*.in"]
        fileTags: ["infile"]
    }
}

// with this https://github.com/qtproject/qt-creator/blob/master/src/app/app_version_header.qbs becomes...
/*
import qbs

Product {
    Depends { name: "configure" }

    name: "app_version_header"
    type: "hpp"
    files: "app_version.h.in"

    configure.replacements: {
        return {
            "IDE_VERSION": project.qtcreator_version,
            "IDE_VERSION_MAJOR": project.ide_version_major,
            "IDE_VERSION_MINOR": project.ide_version_minor,
            "IDE_VERSION_RELEASE": project.ide_version_release
        };
    }

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: product.buildDirectory
    }
}
*/

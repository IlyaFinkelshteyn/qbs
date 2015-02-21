import qbs
import qbs.FileInfo
import qbs.ModUtils
import qbs.TextFile

Module {
    Depends { name: "cpp" }

    additionalProductTypes: ["headersclean.obj"]

    property stringList languages
    PropertyOptions {
        name: "languages"
        description: "languages to compile header files as"
        allowedValues: ["c", "cpp", "objc", "objcpp"]
    }

    // private properties
    readonly property stringList supportedLanguages: ["c", "cpp", "objc", "objcpp"]

    Rule {
        inputs: ["hpp"]
        outputFileTags: ["c", "cpp", "objc", "objcpp", "headersclean.source"]
        outputArtifacts: {
            var artifacts = [];
            var tags = ModUtils.moduleProperty(input, "languages");
            for (var i in tags) {
                if (!ModUtils.moduleProperty(input, "supportedLanguages").contains(tags[i])) {
                    throw "Unsupported language '" + tags[i] + "' for headersclean test";
                }

                var ext = tags[i];
                if (ext === "objc")
                    ext = "m";
                if (ext === "objcpp")
                    ext = "mm";

                var artifact = {
                    filePath: FileInfo.joinPaths(product.buildDirectory, ".headersclean",
                                                 qbs.getHash(input.baseDir),
                                                 input.fileName + "." + ext),
                    fileTags: [tags[i]].concat(["headersclean.source"])
                };
                artifacts.push(artifact);
            }
            return artifacts;
        }

        prepare: {
            var cmds = [];

            var cmd = new JavaScriptCommand();
            var tags = ModUtils.moduleProperty(input, "languages").join(", ");
            cmd.description = "headersclean " + input.fileName + " (" + tags + ")";
            cmd.sourceCode = function() { };
            cmds.push(cmd);

            for (var i in outputs["headersclean.source"]) {
                cmd = new JavaScriptCommand();
                cmd.silent = true;
                var ftags = outputs["headersclean.source"][i].fileTags;
                cmd.objc = ftags.contains("objc") || ftags.contains("objcpp");
                cmd.inputFilePath = input.filePath;
                cmd.outputFilePath = outputs["headersclean.source"][i].filePath;
                cmd.sourceCode = function() {
                    var tf = new TextFile(outputFilePath, TextFile.WriteOnly);
                    var include = objc ? "import" : "include"; // for extremely pedantic warnings
                    tf.writeLine('#' + include + ' "' + inputFilePath + '"');
                    tf.close();
                };
                cmds.push(cmd);
            }

            return cmds;
        }
    }
}

import qbs

Module {
    property string prop
    property stringList listProp: []

    Rule {
        inputs: ["dep-in"]
        Artifact {
            filePath: "dummy.txt"
            fileTags: ["dep-out"]
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "Creating dep-out artifact";
            cmd.sourceCode = function() {
                console.info("prop: " + product.moduleProperty("depmodule", "prop"));
                console.info("listProp: " + product.moduleProperty("depmodule", "listProp"));
            };
            return [cmd];
        }
    }
}

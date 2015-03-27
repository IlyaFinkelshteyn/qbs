import qbs

Product {
    Depends { name: "cpp" }
    type: ["x"]

    Rule {
        inputs: ["qbs"]
        Artifact {
            filePath: "x"
            fileTags: ["x"]
        }
        prepare: {
            var cmd = new Command("/usr/bin/env");
            cmd.silent = true;
            return cmd;
        }
    }
}

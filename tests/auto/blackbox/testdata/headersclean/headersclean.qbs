import qbs

Project {
    CppApplication {
        Depends { name: "headersclean" }

        name: "headersclean-test"

        headersclean.languages: ["c", "cpp", "objc"]

        files: ["*.h", "*.cpp"]
    }

    Product {
        Depends { name: "headersclean" }

        name: "headersclean-test2"

        headersclean.languages: ["c", "cpp", "objc"]

        files: ["*.h"]
    }
}

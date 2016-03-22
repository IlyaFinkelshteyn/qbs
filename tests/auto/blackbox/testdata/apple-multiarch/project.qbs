import qbs

Project {
    CppApplication {
        Depends { name: "multilib" }
        name: "multiapp"
        files: ["app.c"]
        cpp.rpaths: qbs.targetOS.contains("darwin") ? ["@loader_path/../../../"] : []

        Group {
            condition: profile === project.profile
            fileTagsFilter: product.type.concat(["debuginfo", "debuginfo_plist"])
            qbs.install: true
            qbs.installSourceBase: product.buildDirectory
        }
    }

    DynamicLibrary {
        Depends { name: "cpp" }
        name: "multilib"
        files: ["lib.c"]
        cpp.installNamePrefix: qbs.targetOS.contains("darwin") ? "@rpath" : undefined

        Group {
            condition: profile === project.profile
            fileTagsFilter: product.type.concat(["debuginfo", "debuginfo_plist"])
            qbs.install: true
            qbs.installSourceBase: product.buildDirectory
        }
    }
}

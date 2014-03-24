// base for D modules

Module {
    condition: false
    property string warningLevel : 'all' // 'none', 'all'
    property bool treatWarningsAsErrors : false
    property string architecture: qbs.architecture
    property string optimization: qbs.optimization
    property bool debugInformation: qbs.debugInformation

    property pathList importPaths
    property pathList libraryPaths
    property pathList frameworkPaths
    property pathList systemFrameworkPaths
    property pathList linkerScripts
    property string compilerName
    property string compilerPath: compilerName
    property stringList compilerWrapper
    property string linkerName
    property string linkerPath: linkerName
    property string staticLibraryPrefix
    property string dynamicLibraryPrefix
    property string executablePrefix
    property string staticLibrarySuffix
    property string dynamicLibrarySuffix
    property stringList supportedStaticLibrarySuffixes: [staticLibrarySuffix]
    property string executableSuffix
    property bool createSymlinks: true
    property stringList dynamicLibraries // list of names, will be linked with -lname
    property stringList staticLibraries // list of static library files
    property stringList rpaths

    property stringList dFlags
    PropertyOptions {
        name: "dFlags"
        description: "additional flags for the D compiler"
    }

    property stringList linkerFlags
    PropertyOptions {
        name: "linkerFlags"
        description: "additional linker flags"
    }

    property bool positionIndependentCode
    PropertyOptions {
        name: "positionIndependentCode"
        description: "generate position independent code"
    }

    // Platform properties. Those are intended to be set by the toolchain setup
    // and are prepended to the corresponding user properties.
    property stringList platformDFlags
    property stringList platformLinkerFlags

    property bool allowUnresolvedSymbols: false

    FileTagger {
        patterns: ["*.d"]
        fileTags: ["d"]
    }
}

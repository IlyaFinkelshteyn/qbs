import qbs

Project {
    DynamicLibrary {
        name: "lib1"
        files: ["lib1.cpp"]
        Android.ndk.appStl: "stlport_shared"
        cpp.useRPaths: false
    }

    DynamicLibrary {
        name: "lib2"
        files: ["lib2.cpp"]
        Android.ndk.appStl: "stlport_shared"
        cpp.useRPaths: false
    }

    JavaJarFile {
        name: "lib3"
        files: ["lib3.java"]
    }

    AndroidApk {
        name: "twolibs"
        packageName: "io.qt.dummy"
        sourcesDir: "src"
        Depends { productTypes: ["android.nativelibrary", "java.jar"] }
    }
}

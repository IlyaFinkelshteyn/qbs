import qbs

CppApplication {
    condition: qbs.hostOS.contains("osx")
    name: "RezDoc"
    files: ["main.m"]
    cpp.frameworks: ["Cocoa"]

    consoleApplication: true
    destinationDirectory: "bin"

    Group {
        fileTagsFilter: product.type
        qbs.install: true
        qbs.installDir: product.destinationDirectory
    }
}

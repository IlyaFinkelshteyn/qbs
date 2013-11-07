import qbs

Module {
    condition: true

    property string destination
    PropertyOptions {
        name: "destination"
        description: "location within the bundle to copy files into"
        allowedValues: ["Wrapper", "Executables", "Resources", "JavaResources",
                        "Frameworks", "SharedFrameworks", "SharedSupport",
                        "PlugIns", "XPCServices"]
    }

    property string subpath
    PropertyOptions {
        name: "subpath"
        description: "subdirectory within the destination to copy files into"
    }

    property string localization
    PropertyOptions {
        name: "localization"
        description: "localization of the resource; undefined indicates an unlocalized resource"
    }
}

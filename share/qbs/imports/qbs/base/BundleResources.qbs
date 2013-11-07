import qbs

Item {
    Depends { name: "bundle" }

    property string name
    property pathList files
    property string prefix
    property bool condition: true
    property pathList excludeFiles

    property string destination
    property string subpath
    property string localization

    Group {
        bundle.destination: parent.destination
        bundle.subpath: parent.subpath
        bundle.localization: parent.localization
        name: parent.name
        files: parent.files
        prefix: parent.prefix
        condition: parent.condition
        fileTags: ["bundle_resource"]
        excludeFiles: parent.excludeFiles
    }
}

import qbs

Project {
    property bool includeHeaders: true
    Library {
        Depends { name: "cpp" }

        name: "Widget"
        bundle.isBundle: true
        bundle.publicHeaders: project.includeHeaders ? ["Widget.h"] : undefined
        bundle.privateHeaders: project.includeHeaders ? ["WidgetPrivate.h"] : base
        bundle.resources: ["BaseResource", "en.lproj/EnglishResource"]
        files: ["Widget.cpp"]

        Group {
            condition: project.includeHeaders
            files: ["Widget.h", "foo/other.h"]
            fileTags: ["hpp", "hpp_public"]
            qbs.install: !bundle.isBundle
            qbs.installDir: "include"
            qbs.installSourceBase: "."
        }

        Group {
            condition: project.includeHeaders
            files: ["WidgetPrivate.h"]
            fileTags: ["hpp", "hpp_private"]
            qbs.install: !bundle.isBundle
            qbs.installDir: "include"
            qbs.installSourceBase: "."
        }
    }
}

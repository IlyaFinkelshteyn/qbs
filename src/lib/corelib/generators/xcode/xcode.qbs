import qbs

QbsLibrary {
    type: ["staticlibrary"]
    name: "xcodegenerator"
    install: false

    cpp.includePaths: base.concat([
        ".",
        "../..",
    ])

    Depends { name: "cpp" }
    Depends { name: "Qt.core" }

    Group {
        name: "Xcodebuild Object Model"
        prefix: "pbx/"
        files: [
            "pbx.h",
            "pbxbuildfile.cpp",
            "pbxbuildfile.h",
            "pbxbuildphase.cpp",
            "pbxbuildphase.h",
            "pbxbuildrule.cpp",
            "pbxbuildrule.h",
            "pbxcontainer.cpp",
            "pbxcontainer.h",
            "pbxcontaineritemproxy.cpp",
            "pbxcontaineritemproxy.h",
            "pbxfileencoding.h",
            "pbxfilereference.cpp",
            "pbxfilereference.h",
            "pbxfiletype.cpp",
            "pbxfiletype.h",
            "pbxframeworksbuildphase.cpp",
            "pbxframeworksbuildphase.h",
            "pbxgroup.cpp",
            "pbxgroup.h",
            "pbxlegacytarget.cpp",
            "pbxlegacytarget.h",
            "pbxnativetarget.cpp",
            "pbxnativetarget.h",
            "pbxobject.cpp",
            "pbxobject.h",
            "pbxproducttype.cpp",
            "pbxproducttype.h",
            "pbxproject.cpp",
            "pbxproject.h",
            "pbxreference.cpp",
            "pbxreference.h",
            "pbxsourcesbuildphase.cpp",
            "pbxsourcesbuildphase.h",
            "pbxsourcetree.cpp",
            "pbxsourcetree.h",
            "pbxtarget.cpp",
            "pbxtarget.h",
            "pbxtargetdependency.cpp",
            "pbxtargetdependency.h",
            "xcbuildconfiguration.cpp",
            "xcbuildconfiguration.h",
            "xcconfigurationlist.cpp",
            "xcconfigurationlist.h",
        ]
    }

    Group {
        name: "Xcodebuild Object Model I/O"
        prefix: "io/"
        files: [
            "opensteppropertylist.cpp",
            "opensteppropertylist.h",
            "pbxprojectwriter.cpp",
            "pbxprojectwriter.h",
        ]
    }

    Group {
        name: "Xcode Configuration Object Model"
        prefix: "xcconfig/"
        files: [
            "xcconfig.cpp",
            "xcconfig.h",
        ]
    }

    Group {
        name: "Xcode Scheme Object Model"
        prefix: "xcscheme/"
        files: [
            "xcscheme.cpp",
            "xcscheme.h",
        ]
    }

    Group {
        name: "Xcode Settings Object Model"
        prefix: "xcsettings/"
        files: [
            "xcsettings.cpp",
            "xcsettings.h",
        ]
    }

    Group {
        name: "Xcode generator"
        files: [
            "xcodeframeworksbuildphase.cpp",
            "xcodeframeworksbuildphase.h",
            "xcodegenerator.cpp",
            "xcodegenerator.h",
            "xcodegeneratorutils.cpp",
            "xcodegeneratorutils.h",
            "xcodenativegenerator.cpp",
            "xcodenativegenerator.h",
            "xcodeschemesvisitor.cpp",
            "xcodeschemesvisitor.h",
            "xcodesimplegenerator.cpp",
            "xcodesimplegenerator.h",
            "xcodebuildsettingmapping.cpp",
            "xcodebuildsettingmapping.h",
            "xcodesourcesbuildphase.cpp",
            "xcodesourcesbuildphase.h",
        ]
    }
}

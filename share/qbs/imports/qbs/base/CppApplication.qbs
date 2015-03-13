import qbs 1.0

Application {
    Depends { name: "cpp" }

    property bool install: false
    property string installDir: bundle.isBundle ? qbs.applicationBundleInstallDir : qbs.binInstallDir

    Group {
        fileTagsFilter: ["application"]
        qbs.install: install
        qbs.installDir: FileInfo.joinPaths(installDir, bundle.isBundle
                                           ? FileInfo.path(bundle.executablePath)
                                           : undefined)
    }

    Group {
        fileTagsFilter: ["infoplist"]
        qbs.install: install && bundle.isBundle
        qbs.installDir: FileInfo.joinPaths(installDir, FileInfo.path(bundle.infoPlistPath))
    }

    Group {
        fileTagsFilter: ["pkginfo"]
        qbs.install: install && bundle.isBundle
        qbs.installDir: FileInfo.joinPaths(installDir, FileInfo.path(bundle.pkgInfoPath))
    }
}


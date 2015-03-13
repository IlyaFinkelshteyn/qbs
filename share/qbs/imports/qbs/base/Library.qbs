Product {
    type: {
        if (qbs.targetOS.contains("ios") && parseInt(cpp.minimumIosVersion, 10) < 8)
            return ["staticlibrary"];
        return ["dynamiclibrary"].concat(isForAndroid ? ["android.nativelibrary"] : []);
    }

    property bool isForAndroid: qbs.targetOS.contains("android")
    property stringList architectures: isForAndroid ? ["armeabi"] : undefined

    Depends { name: "Android.ndk"; condition: isForAndroid }
    Depends { name: "bundle" }
    Depends { name: "cpp"; condition: isForAndroid }

    profiles: isForAndroid
        ? architectures.map(function(arch) { return project.profile + '_' + arch; })
        : [project.profile]

    property bool install: false
    property string installDir: bundle.isBundle ? qbs.frameworkInstallDir : qbs.libInstallDir

    Group {
        fileTagsFilter: ["staticlibrary", "dynamiclibrary", "dynamiclibrary_symlink"]
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

    Group {
        fileTagsFilter: ["bundle.symlink.headers", "bundle.symlink.private-headers",
            "bundle.symlink.resources", "bundle.symlink.executable"]
        qbs.install: install && bundle.isBundle
        qbs.installDir: FileInfo.joinPaths(installDir, bundle.bundleName)
    }

    Group {
        fileTagsFilter: ["bundle.symlink.version"]
        qbs.install: install && bundle.isBundle
        qbs.installDir: FileInfo.joinPaths(installDir, bundle.bundleName, "Versions")
    }

    Group {
        fileTagsFilter: ["bundle.hpp.public"]
        qbs.install: install && bundle.isBundle
        qbs.installDir: FileInfo.joinPaths(installDir, bundle.publicHeadersFolderPath)
        // TODO: non-bundle: "include/" + product.targetName?
    }

    Group {
        fileTagsFilter: ["bundle.hpp.private"]
        qbs.install: install && bundle.isBundle
        qbs.installDir: FileInfo.joinPaths(installDir, bundle.privateHeadersFolderPath)
        // TODO: non-bundle: "include/" + product.targetName?
    }

    Group {
        fileTagsFilter: ["bundle.resource"]
        qbs.install: install && bundle.isBundle
        qbs.installDir: FileInfo.joinPaths(installDir, bundle.unlocalizedResourcesFolderPath)
    }
}

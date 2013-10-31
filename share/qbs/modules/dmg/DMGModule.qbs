import qbs 1.0
import qbs.FileInfo
import qbs.Process
import qbs.TextFile
import "../cpp/darwin-tools.js" as DarwinTools
import "rezutils.js" as RezUtils
import "../utils.js" as ModUtils

Module {
    condition: qbs.hostOS.contains("darwin") && qbs.targetOS.contains("darwin")

    property string volumeName: product.targetName
    PropertyOptions {
        name: "volumeName"
        description: "the name of the disk image (displayed in Finder when mounted)"
    }

    property path volumeIcon
    PropertyOptions {
        name: "volumeIcon"
        description: "path to an .icns file to use for the root of the disk image"
    }

    property path dsStoreFile
    PropertyOptions {
        name: "dsStoreFile"
        description: "path to the .DS_Store file to copy to the root of the disk image"
    }

    property bool createDSStore: dsStoreFile === undefined
    PropertyOptions {
        name: "createDSStore"
        description: "whether to synthesize a .DS_Store file from various property values"
    }

    property path backgroundImage
    PropertyOptions {
        name: "backgroundImage"
        description: "path to a TIFF-format background image file to be used for the disk image"
    }

    property var licenses
    PropertyOptions {
        name: "licenses"
        description: "list of dictionaries containing the configuration of licenses displayed " +
                     "when opening the disk image; this can be used instead of licenseFiles if " +
                     "your project does not store license files in lproj folders or needs to " +
                     "specify additional information, such as the button localizations"
    }

    property pathList licenseFiles
    PropertyOptions {
        name: "licenseFiles"
        description: "paths of license files displayed when opening the disk image; " +
                     "locales are determined by examining the file paths for .lproj components"
    }

    property string format: "UDBZ"
    PropertyOptions {
        name: "format"
        allowedValues: ["UDZO", "UDBZ"]
        description: "the format to create the disk image in"
    }

    property int compressionLevel: qbs.buildVariant === "release" ? 9 : undefined
    PropertyOptions {
        name: "compressionLevel"
        description: "sets the zlib or bzip2 compression level for UDZO and UDBZ disk images"
    }

    property bool internetEnable: false
    PropertyOptions {
        name: "internetEnable"
        description: "whether to Internet-enable the disk image"
    }

    property pathList files: []
    PropertyOptions {
        name: "files"
        description: "list of files to copy to the disk image"
    }

    property pathList directories: []
    PropertyOptions {
        name: "directories"
        description: "list of directories to copy to the disk image"
    }

    property pathList symlinks: ["/Applications:Applications"]
    PropertyOptions {
        name: "symlinks"
        description: "list of symbolic links to create in the disk image"
    }

    property pathList resourceFiles: []
    PropertyOptions {
        name: "resourceFiles"
        description: "list of additional resource files (.r) to merge into the disk image"
    }

    property var extraStagingCommands: []
    PropertyOptions {
        name: "extraStagingCommands"
        description: "list of additional commands to execute when setting up the staging " +
                     "directory to create the disk image from - this is a good place to run " +
                     "a tool like macdeployqt for example"
    }

    property string dmgSuffix: ".dmg"

    validate: {
        if (!qbs.sysroot) {
            // Rez needs a sysroot to find Carbon.r as of OS X 10.9
            throw "qbs.sysroot is not defined. Set qbs.sysroot in your profile.";
        }
    }

    Rule {
        multiplex: true
        inputs: ["qbs"]

        Artifact {
            fileTags: ["dmg"]
            fileName: product.destinationDirectory + "/" + product.targetName +
                      ModUtils.moduleProperty(product, "dmgSuffix")
        }

        prepare: {
            var i;
            var cmd;
            var cmds = [];

            var createDSStore = ModUtils.moduleProperty(product, "createDSStore");
            var dsStoreFile = ModUtils.moduleProperty(product, "dsStoreFile");
            if (createDSStore && dsStoreFile) {
                throw("createDSStore and dsStoreFile are mutually exclusive");
            }

            if (licenses && typeof licenses !== "Array") {
                throw("licenses must be an array");
            }

            var licenses = ModUtils.moduleProperty(product, "licenses");
            var licenseFiles = ModUtils.moduleProperty(product, "licenseFiles");
            if (licenses.length > 0 && licenseFiles.length > 0) {
                throw("licenses and licenseFiles are mutually exclusive");
            }

            var backgroundImage = ModUtils.moduleProperty(product, "backgroundImage");
            if (backgroundImage && !createDSStore && !dsStoreFile) {
                throw("either createDSStore or dsStoreFile must be set to use backgroundImage");
            }

            // Synthesize a .DS_Store file...
            if (createDSStore) {
                // Write a .DS_Store file to here
                //FileInfo.joinPaths(stageDirectory, ".DS_Store")
                throw(".DS_Store synthesis is not yet supported.");
            }

            var stageDirectory = FileInfo.joinPaths(output.fileName + "-tmp", "stage");

            // List of source/destination paths to copy with rsync
            var rsyncPaths = [];

            var files = ModUtils.moduleProperty(product, "files");
            for (i in files) {
                var sourceFile = files[i].split(':')[0];
                var targetDirectory = stageDirectory;

                var targetSubpath = files[i].split(':')[1];
                if (targetSubpath) {
                    if (targetSubpath[0] === "/")
                        throw(files[i] + " is an invalid copy command; " +
                              "the destination directory must be a relative path");
                    targetDirectory = FileInfo.joinPaths(stageDirectory, targetSubpath);
                }

                rsyncPaths.push({ source: sourceFile,
                                  destination: FileInfo.joinPaths(targetDirectory,
                                                                  FileInfo.fileName(files[i])) });
            }

            var directories = ModUtils.moduleProperty(product, "directories");
            for (i in directories) {
                var sourceDirectory = directories[i].split(':')[0];
                var targetDirectory = stageDirectory;

                var targetSubpath = directories[i].split(':')[1];
                if (targetSubpath) {
                    if (targetSubpath[0] === "/")
                        throw(directories[i] + " is an invalid copy command; " +
                              "the destination directory must be a relative path");
                    targetDirectory = FileInfo.joinPaths(stageDirectory, targetSubpath);
                }

                rsyncPaths.push({ source: sourceDirectory,
                                  destination: targetDirectory, isDirectory: true });
            }

            if (backgroundImage) {
                rsyncPaths.push({ source: backgroundImage,
                                  destination: FileInfo.joinPaths(stageDirectory, ".background",
                                                                  "backgroundImage.tiff") });
            }

            var volumeIcon = ModUtils.moduleProperty(product, "volumeIcon");
            if (volumeIcon) {
                if (FileInfo.extension(volumeIcon) !== "icns")
                    throw("volumeIcon must be a .icns file");
                rsyncPaths.push({ source: volumeIcon,
                                  destination: FileInfo.joinPaths(stageDirectory, ".VolumeIcon.icns") });
            }

            if (dsStoreFile) {
                rsyncPaths.push({ source: dsStoreFile,
                                  destination: FileInfo.joinPaths(stageDirectory, ".DS_Store") });
            }

            // Copy relevant files and directories to the disk image staging directory
            for (i in rsyncPaths) {
                var rsync = rsyncPaths[i];
                cmds.push(new Command("mkdir", ["-p", rsync.isDirectory
                                                ? rsync.destination : FileInfo.path(rsync.destination)]));
                cmds.push(new Command("rsync", ["-a", "--copy-unsafe-links",
                                                rsync.source, rsync.destination]));
            }

            // Make links
            var symlinks = ModUtils.moduleProperty(product, "symlinks")
            for (i in symlinks) {
                var symlinkTarget = symlinks[i].split(':')[0];
                var symlinkName = symlinks[i].split(':')[1] || symlinkTarget;

                if (symlinkName[0] === "/") {
                    throw(symlinks[i] + " is an invalid symlink; " +
                          "the destination must be a relative path");
                }

                cmd = new Command("ln", ["-s", symlinkTarget, FileInfo.joinPaths(stageDirectory,
                                                                                 symlinkName)]);
                cmd.workingDirectory = stageDirectory;
                cmds.push(cmd);
            }

            // Make background folder invisible
            if (backgroundImage) {
                cmds.push(new Command("SetFile", ["-a", "V", FileInfo.joinPaths(stageDirectory,
                                                                                ".background")]));
            }

            // Set the custom icon attribute on the root directory
            if (volumeIcon) {
                cmds.push(new Command("SetFile", ["-a", "C", stageDirectory]));
            }

            var extraStagingCommands = ModUtils.moduleProperty(product, "extraStagingCommands");
            if (!(extraStagingCommands instanceof Array)) {
                throw("extraStagingCommands must be an array of non-empty string arrays");
            }

            for (i in extraStagingCommands) {
                var command = extraStagingCommands[i];
                if (!(command instanceof Array) || command.length == 0) {
                    throw("extraStagingCommands must be an array of non-empty string arrays");
                }

                cmd = new Command(command[0], command.slice(1));
                cmd.workingDirectory = stageDirectory;
                cmds.push(cmd);
            }

            // Make all files only: world-readable, owner-writable and world-executable
            cmds.push(new Command("chmod", ["-R", "a+rX,a-st,u+w,go-w", stageDirectory]));

            // Create a temporary uncompressed disk image from the staging directory...
            var volumeName = ModUtils.moduleProperty(product, "volumeName");
            var hybridDmg = FileInfo.joinPaths(FileInfo.path(stageDirectory), "hybrid.dmg");
            cmd = new Command("hdiutil", ["makehybrid", "-quiet", "-hfs", "-hfs-volume-name",
                                          volumeName, "-hfs-openfolder", stageDirectory, "-ov",
                                          stageDirectory, "-o", hybridDmg]);
            cmds.push(cmd);

            // Remove the staging directory
            cmds.push(new Command("rm", ["-rf", stageDirectory]));

            // Convert the uncompressed image to the correct format
            var destinationFormat = ModUtils.moduleProperty(product, "format");
            var convertArgs = ["convert", "-quiet", "-format", destinationFormat];

            var compressionLevel = ModUtils.moduleProperty(product, "compressionLevel");
            if (compressionLevel !== undefined) {
                if (destinationFormat === "UDZO") {
                    convertArgs.push("-imagekey");
                    convertArgs.push("zlib-level=" + compressionLevel);
                } else if (destinationFormat === "UDBZ") {
                    convertArgs.push("-imagekey");
                    convertArgs.push("bzip2-level=" + compressionLevel);
                } else {
                    throw("Compression is not supported for " + destinationFormat + "-formatted images");
                }
            }

            cmd = new Command("hdiutil", convertArgs.concat(["-ov", hybridDmg, "-o", output.fileName]));
            cmd.description = "building " + FileInfo.fileName(output.fileName);
            cmd.highlight = "compiler";
            cmds.push(cmd);

            // Get rid of the original image
            cmds.push(new Command("rm", ["-f", hybridDmg]));

            // Extract UDIF metadata into the resource fork
            cmds.push(new Command("hdiutil", ["unflatten", "-quiet", output.fileName]));

            var resourceFiles = ModUtils.moduleProperty(product, "resourceFiles");

            // Determine the localizations of the listed license files...
            if (licenseFiles.length > 0) {
                licenses = [];
                for (i in licenseFiles) {
                    var locale = DarwinTools.localizationKey(licenseFiles[i]);
                    if (locale === undefined) {
                        throw("Could not determine localization for license file " + licenseFiles[i]);
                    }
                    licenses.push({
                        "Locale": locale,
                        "Filename": licenseFiles[i]
                    });
                }
            }

            // ### TODO: QBS-436: Find a cleaner way to do this!
            var process = new Process();
            process.exec("which", ["qbs"], true);
            var modulesDirectory = FileInfo.joinPaths(FileInfo.path(process.readStdOut()),
                                                      "../share/qbs/modules");

            // Synthesize license files
            if (licenses.length > 0) {
                var licenseResource = FileInfo.joinPaths(FileInfo.path(stageDirectory), "license.r");
                resourceFiles.push(licenseResource);

                var args = ["-l", "-o", licenseResource];
                for (i = 0; i < licenses.length; ++i) {
                    args.push(licenses[i]["Locale"]);
                    args.push(licenses[i]["Filename"]);
                    resourceFiles.push(FileInfo.joinPaths(modulesDirectory, product.moduleName,
                                                          "license-" + licenses[i]["Locale"] + ".r"));
                }
                cmds.push(new Command("RezDoc", args));
            }

            // Merge resource files
            for (i in resourceFiles) {
                cmds.push(new Command("Rez", ["-is", product.moduleProperty("qbs", "sysroot"),
                                              "Carbon.r", resourceFiles[i], "-a", "-o", output.fileName]));
            }

            // Embed resource fork into UDIF data fork
            cmds.push(new Command("hdiutil", ["flatten", "-quiet", output.fileName]));

            // Remove the base working directory
            cmds.push(new Command("rm", ["-rf", FileInfo.path(stageDirectory)]));

            // "Internet Enable" the disk image
            if (ModUtils.moduleProperty(product, "internetEnable")) {
                cmds.push(new Command("hdiutil", ["internet-enable", "-quiet", "-yes", output.fileName]));
            }

            return cmds;
        }
    }
}

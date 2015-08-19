/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing
**
** This file is part of the Qt Build Suite.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms and
** conditions see http://www.qt.io/terms-conditions. For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, The Qt Company gives you certain additional
** rights.  These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

//var File = loadExtension("qbs.File");
import FileInfo = require("../../../imports/qbs/FileInfo/fileinfo");
//var TextFile = loadExtension("qbs.TextFile");

export function sourceAndTargetFilePathsFromInfoFiles(inputs: ArtifactMap, product: Product, inputTag: string) {
    var sourceFilePaths: string[] = [];
    var targetFilePaths: string[] = [];
    var inputsLength = inputs[inputTag] ? inputs[inputTag].length : 0;
    for (var i = 0; i < inputsLength; ++i) {
        var infoFile = new TextFile(inputs[inputTag][i].filePath, TextFile.ReadOnly);
        var sourceFilePath = infoFile.readLine();
        var targetFilePath = FileInfo.joinPaths(product.buildDirectory, infoFile.readLine());
        if (!targetFilePaths.contains(targetFilePath)) {
            sourceFilePaths.push(sourceFilePath);
            targetFilePaths.push(targetFilePath);
        }
        infoFile.close();
    }
    return { sourcePaths: sourceFilePaths, targetPaths: targetFilePaths };
}

export function outputArtifactsFromInfoFiles(inputs: ArtifactMap, product: Product, inputTag: string, outputTag: string) {
    var pathSpecs = sourceAndTargetFilePathsFromInfoFiles(inputs, product, inputTag)
    var artifacts: Artifact[] = [];
    for (var i = 0; i < pathSpecs.targetPaths.length; ++i)
        artifacts.push({ filePath: pathSpecs.targetPaths[i], fileTags: [outputTag] });
    return artifacts;
}

export function availableSdkPlatforms(sdkDir: string) {
    var re = /^android-([0-9]+)$/;
    var platforms = File.directoryEntries(FileInfo.joinPaths(sdkDir, "platforms"),
        File.Dirs | File.NoDotAndDotDot);
    var versions = [];
    for (var i = 0; i < platforms.length; ++i) {
        var match = platforms[i].match(re);
        if (match !== null) {
            versions.push(platforms[i]);
        }
    }

    versions.sort(function(a, b) {
        return parseInt(a.match(re)[1], 10) - parseInt(b.match(re)[1], 10);
    });
    return versions;
}

export function availableBuildToolsVersions(sdkDir) {
    var re = /^([0-9]+)\.([0-9]+)\.([0-9]+)$/;
    var buildTools = File.directoryEntries(FileInfo.joinPaths(sdkDir, "build-tools"),
        File.Dirs | File.NoDotAndDotDot);
    var versions = [];
    for (var i = 0; i < buildTools.length; ++i) {
        var match = buildTools[i].match(re);
        if (match !== null) {
            versions.push(buildTools[i]);
        }
    }

    // Sort by version number
    versions.sort(function(a, b) {
        a = a.match(re);
        if (a)
            a = { major: a[1], minor: a[2], patch: a[3] };
        b = b.match(re);
        if (b)
            b = { major: b[1], minor: b[2], patch: a[3] };

        if (a.major === b.major) {
            if (a.minor === b.minor)
                return a.patch - b.patch;
            return a.minor - b.minor;
        }
        return a.major - b.major;
    });

    return versions;
}

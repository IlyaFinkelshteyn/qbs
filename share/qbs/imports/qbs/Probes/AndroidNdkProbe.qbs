/****************************************************************************
**
** Copyright (C) 2015 Jake Petroules.
** Contact: http://www.qt.io/licensing
**
** This file is part of Qbs.
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

import qbs
import qbs.Environment
import qbs.File
import qbs.FileInfo

PathProbe {
    // Inputs
    property stringList hostOS: qbs.hostOS
    property path sdkPath

    environmentPaths: Environment.getEnv("ANDROID_NDK_ROOT")
    platformPaths: {
        var paths = [];
        if (sdkPath)
            paths.push(FileInfo.joinPaths(sdkPath, "ndk-bundle"));
        if (qbs.hostOS.contains("windows"))
            paths.push(FileInfo.joinPaths(Environment.getEnv("LOCALAPPDATA"),
                                          "Android", "sdk", "ndk-bundle"));
        if (qbs.hostOS.contains("macos"))
            paths.push(FileInfo.joinPaths(Environment.getEnv("HOME"),
                                          "Library", "Android", "sdk", "ndk-bundle"));
        if (qbs.hostOS.contains("linux"))
            paths.push(FileInfo.joinPaths(Environment.getEnv("HOME"),
                                          "Android", "Sdk", "ndk-bundle"));
        return paths;
    }

    // Outputs
    property var hostArch
    property stringList toolchains: []

    configure: {
        var i, j, allPaths = (environmentPaths || []).concat(platformPaths || []);
        for (i in allPaths) {
            var platforms = [];
            if (hostOS.contains("windows"))
                platforms.push("windows-x86_64", "windows");
            if (hostOS.contains("darwin"))
                platforms.push("darwin-x86_64", "darwin-x86");
            if (hostOS.contains("linux"))
                platforms.push("linux-x86_64", "linux-x86");
            for (j in platforms) {
                if (File.exists(FileInfo.joinPaths(allPaths[i], "prebuilt", platforms[j]))) {
                    path = allPaths[i];
                    hostArch = platforms[j];
                    toolchains = File.directoryEntries(FileInfo.joinPaths(path, "toolchains"),
                                                       File.Dirs | File.NoDotAndDotDot);
                    found = true;
                    return;
                }
            }
        }
    }
}

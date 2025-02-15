/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

import qbs
import qbs.ModUtils
import "../../../modules/cpp/gcc.js" as Gcc

PathProbe {
    // Inputs
    property string compilerFilePath
    property string preferredArchitecture
    property string preferredMachineType
    property stringList flags: []

    property bool _haveArchFlag: qbs.targetOS.contains("darwin")
    property string _nullDevice: qbs.nullDevice
    property stringList _toolchain: qbs.toolchain
    property string _pathListSeparator: qbs.pathListSeparator
    property stringList _targetOS: qbs.targetOS
    property string _sysroot: qbs.sysroot

    // Outputs
    property string architecture
    property int versionMajor
    property int versionMinor
    property int versionPatch
    property stringList includePaths
    property stringList libraryPaths
    property stringList frameworkPaths

    configure: {
        var args = flags;
        if (_haveArchFlag) {
            if (preferredArchitecture)
                args.push("-arch", preferredArchitecture);
        } else {
            if (preferredArchitecture === "i386")
                args.push("-m32");
            else if (preferredArchitecture === "x86_64")
                args.push("-m64");

            if (preferredMachineType)
                args.push("-march=" + preferredMachineType);
        }

        var macros = Gcc.dumpMacros(compilerFilePath, args, _nullDevice);
        var defaultPaths = Gcc.dumpDefaultPaths(compilerFilePath, args, _nullDevice,
                                                _pathListSeparator, _targetOS, _sysroot);
        found = !!macros && !!defaultPaths;

        includePaths = defaultPaths.includePaths;
        libraryPaths = defaultPaths.libraryPaths;
        frameworkPaths = defaultPaths.frameworkPaths;

        // We have to dump the compiler's macros; -dumpmachine is not suitable because it is not
        // always complete (for example, the subarch is not included for arm architectures).
        architecture = ModUtils.guessArchitecture(macros) || preferredArchitecture;

        if (_toolchain.contains("clang")) {
            versionMajor = parseInt(macros["__clang_major__"], 10);
            versionMinor = parseInt(macros["__clang_minor__"], 10);
            versionPatch = parseInt(macros["__clang_patchlevel__"], 10);
        } else {
            versionMajor = parseInt(macros["__GNUC__"], 10);
            versionMinor = parseInt(macros["__GNUC_MINOR__"], 10);
            versionPatch = parseInt(macros["__GNUC_PATCHLEVEL__"], 10);
        }
    }
}

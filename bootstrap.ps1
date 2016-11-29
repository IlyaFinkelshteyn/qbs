#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of Qbs.
##
## $QT_BEGIN_LICENSE:LGPL$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU Lesser General Public License Usage
## Alternatively, this file may be used under the terms of the GNU Lesser
## General Public License version 3 as published by the Free Software
## Foundation and appearing in the file LICENSE.LGPL3 included in the
## packaging of this file. Please review the following information to
## ensure the GNU Lesser General Public License version 3 requirements
## will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 2.0 or (at your option) the GNU General
## Public license version 3 or any later version approved by the KDE Free
## Qt Foundation. The licenses are as published by the Free Software
## Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-2.0.html and
## https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

#Requires -Version 2

$ErrorActionPreference = "Stop"

try { $QTDIR = $(& qmake -query QT_INSTALL_PREFIX) } catch { }
$QBS_BUILD_VARIANT = "release"
$QBS_INSTALL_ROOT = "build"

$qbsargs = @()
for ($i = 0; $i -lt $args.count; $i++) {
    if (($args[$i] -eq '-h') -or ($args[$i] -eq '-help') -or ($args[$i] -eq '--help')) {
        echo "usage: .\bootstrap [Qt.core.prefixPath:<QTDIR>] [property:value] ..."
        echo
        echo "Performs a bootstrapped build of Qbs."
        echo
        echo "By default, the bootstrapped installation will be placed in <SRCDIR>/build,"
        echo "but this can be overridden by specifying the qbs.installRoot:<DIR> argument."
        echo
        echo "Qt.core.prefixPath:<QTDIR> must be set to a path to a prebuilt Qt installation,"
        echo "unless qmake is in the PATH. Required modules include QtCore and QtScript."
        Exit 2
    } elseif ($args[$i] -like '*:*') {
        $value = $args[$i].Substring($args[$i].IndexOf(':') + 1)
        if ($args[$i] -like 'qbs.buildVariant:*') {
            $QBS_BUILD_VARIANT = $value
        } elseif ($args[$i] -like 'qbs.installRoot:*') {
            $QBS_INSTALL_ROOT = $value
        } elseif ($args[$i] -like 'Qt.core.prefixPath:*') {
            $QTDIR = $value
        } else {
            # forward other properties directly to bootstrapped qbs
            # we exclude all other argument forms because we don't want users
            # to accidentally or intentionally build multiple configurations
            $qbsargs += $value
        }
    } else {
        Write-Error "error: unknown option '$($args[$i])'"
        Exit 1
    }
}

If (!$QTDIR -or !(Test-Path $QTDIR -PathType Container) -or `
    !(Get-Command (Join-Path (Join-Path $QTDIR bin) qmake) -ErrorAction SilentlyContinue)) {
    Write-Error "error: could not find Qt, did you forget to specify Qt.core.prefixPath?"
    Exit 1
}

$QMAKE = Join-Path (Join-Path $QTDIR bin) qmake
$QT_HOST_BINS = & $QMAKE "-query" "QT_HOST_BINS" | `
    % { [System.Io.Path]::GetFullPath($_) }
$QT_HOST_DATA = & $QMAKE "-query" "QT_HOST_DATA" | `
    % { [System.Io.Path]::GetFullPath($_) }
$QT_INSTALL_BINS = & $QMAKE "-query" "QT_INSTALL_BINS" | `
    % { [System.Io.Path]::GetFullPath($_) }
$QT_INSTALL_HEADERS = & $QMAKE "-query" "QT_INSTALL_HEADERS" | `
    % { [System.Io.Path]::GetFullPath($_) }
$QT_INSTALL_LIBS = & $QMAKE "-query" "QT_INSTALL_LIBS" | `
    % { [System.Io.Path]::GetFullPath($_) }
$QT_MKSPEC = & $QMAKE "-query" "QMAKE_XSPEC"

$cflags = @()
$lflags = @()
$mocflags = @()

$qconfig_content = Get-Content (Join-Path (Join-Path $QT_HOST_DATA mkspecs) qconfig.pri)
If ($qconfig_content -match '\bqt_framework\b') {
    $cflags += @("-DQT_CORE_LIB", "-DQT_SCRIPT_LIB")
    $cflags += "-I$QT_INSTALL_HEADERS"
    $cflags += "-F$QT_INSTALL_LIBS"
    $lflags += "-L$QT_INSTALL_LIBS"
    $lflags += @("-framework", "QtCore", "-framework", "QtScript")
    # Relocatability is not relevant for the bootstrapped binary
    $lflags += @("-Xlinker", "-rpath", "$QT_INSTALL_LIBS")
} ElseIf ($QT_MKSPEC -like '*msvc*') {
    $cflags += @("/DQT_CORE_LIB", "/DQT_SCRIPT_LIB")
    $cflags += "/I$QT_INSTALL_HEADERS"
    $lflags += @(
        (Join-Path $QT_INSTALL_LIBS Qt5Core.lib),
        (Join-Path $QT_INSTALL_LIBS Qt5Script.lib)
    )
} Else {
    $cflags += @("-DQT_CORE_LIB", "-DQT_SCRIPT_LIB")
    $cflags += "-I$QT_INSTALL_HEADERS"
    $lflags += "-L$QT_INSTALL_LIBS"
    $lflags += @("-lQt5Core", "-lQt5Script")
}

$mocflags += @("-DQT_CORE_LIB", "-DQT_SCRIPT_LIB")

# QTBUG-34593, QTBUG-52687
If ("$QT_MKSPEC" -like '*macx*') {
    $mocflags += @("-DQ_OS_DARWIN", "-DQ_OS_MAC")
}

$tmpdir = New-Item -ItemType Directory `
    -Path (Join-Path ([System.IO.Path]::GetTempPath()) ([System.IO.Path]::GetRandomFileName()))

# Collect qbs source files and include directories
$pwd = (Resolve-Path .).Path
$source_paths = @(
    (Join-Path $pwd (Join-Path src (Join-Path app qbs))),
    (Join-Path $pwd (Join-Path src (Join-Path app (Join-Path shared logging)))),
    (Join-Path $pwd (Join-Path src (Join-Path lib corelib))),
    (Join-Path $pwd (Join-Path src plugins))
)
$exclude_patterns = @(
    "changeset.*", "projectfileupdater.*", "qmljsrewriter.*",
    "domxml.*", "propertylist*",
    "applecodesignutils.*",
    "tst_*"
)
$source_files = @()
$objc_source_files = @()
$header_files = @()
$include_dirs = @($tmpdir, $pwd, (Join-Path $pwd src)) + $source_paths
foreach ($source_path in $source_paths) {
    $source_files += Get-ChildItem -Filter *.cpp -Path $source_path -Recurse `
        -Exclude $exclude_patterns | Where-Object { !$_.PSIsContainer }
    $header_files += Get-ChildItem -Filter *.h -Path $source_path -Recurse `
        -Exclude $exclude_patterns | Where-Object { !$_.PSIsContainer }
    $include_dirs += Get-ChildItem -Path $source_path -Recurse `
        -Exclude $exclude_patterns | Where-Object { $_.PSIsContainer }
}

$jsextensions = "src/lib/corelib/jsextensions"
if ($QT_MKSPEC -like '*macx*') {
    $header_files += @("$jsextensions/propertylistutils.h")
    $objc_source_files += @("$jsextensions/propertylist.mm", "$jsextensions/propertylistutils.mm")
    $lflags += @("-framework", "CoreFoundation", "-framework", "Foundation")
} Else {
    $source_files += @(Join-Path $jsextensions "propertylist.cpp")
}

# Build compiler and linker flag lists
$qbsversion = Select-String -Path qbs-resources/imports/QbsFunctions/functions.js `
    -Pattern '\".*\"' -AllMatches | % { $_.Matches } | % { $_.Value }
$defines = @(
    "#define QBS_VERSION $qbsversion",
    "#define SRCDIR '$pwd'".Replace("'", '"').Replace("\", "/"),
    "#define QBS_BOOTSTRAPPED",
    "#define QBS_STATIC_LIB",
    "#define CPLUSPLUS_NO_PARSER"
)
If ($QT_MKSPEC -like '*win32*') {
    $defines += @(
        "#define UNICODE",
        "#define _UNICODE",
        "#define WINVER 0x0601",
        "#define _WIN32_WINNT 0x0601"
    )
}
$defines_file = Join-Path $tmpdir qbs-defines.h
Set-Content $defines_file $defines

If ($QT_MKSPEC -like '*win32*') {
    $outexe = Join-Path $tmpdir "qbs.exe"
} Else {
    $outexe = Join-Path $tmpdir "qbs"
}

If ($QT_MKSPEC -like '*msvc*') {
    $cflags += @("/nologo", "/EHsc", "/FI$defines_file",
        "/Fo$tmpdir$([IO.Path]::DirectorySeparatorChar)")
    $lflags = @("/link", "/OUT:$outexe", "psapi.lib", "shell32.lib") + $lflags
} Else {
    $cflags += @("-pipe", "-std=c++11", "-include", "$defines_file")
    If ($QT_MKSPEC -like 'win32-g++') {
        $lflags += @("-lpsapi", "-lshell32")
    } Else {
        $cflags += "-fPIC"
    }
    $lflags += @("-o", $outexe)
}

foreach ($dir in $include_dirs) {
    If ($dir) {
        If ($QT_MKSPEC -like '*msvc*') {
            $cflags += "/I$dir"
        } Else {
            $cflags += "-I$dir"
        }
    }
}

# Run moc
$all_files = $header_files + $source_files + $objc_source_files
foreach ($src in $all_files) {
    If ((Get-Content $src) -match '#include.*\.moc"') {
        $fname = "$((Get-Item $src).BaseName).moc"
    } ElseIf ((Get-Content $src) -match '^ *\bQ_OBJECT\b') {
        $fname = "moc_$((Get-Item $src).BaseName).cpp"
        $source_files += Join-Path $tmpdir $fname
    } Else {
        Continue
    }
    (& (Join-Path $QT_HOST_BINS moc) $mocflags $src) | Set-Content (Join-Path $tmpdir $fname)
}

# Create amalgamation files to speed up compilation
$qbs_amalgamation = Join-Path $tmpdir qbs.cpp
foreach ($src in $source_files) {
    Add-Content $qbs_amalgamation "`n#include `"$src`""
}

if ($QT_MKSPEC -like '*macx*') {
    $qbs_amalgamation_objc = Join-Path $tmpdir qbs.mm
    foreach ($src in $objc_source_files) {
        Add-Content $qbs_amalgamation_objc "`n#include `"$src`""
    }
}

# Build the bootstrap binary
$response_file = Join-Path $tmpdir resp
Set-Content $response_file ($cflags + $qbs_amalgamation + $qbs_amalgamation_objc)
if ($QT_MKSPEC -eq "win32-g++") {
    # GCC cannot understand backslashes in paths on Windows
    (Get-Content $response_file) -replace "\\", "/" | Set-Content $response_file
}
if (($QT_MKSPEC -like 'win32-msvc*') -and (Get-Command cl -ErrorAction SilentlyContinue)) {
    cl "@$response_file" $lflags
} ElseIf (!($QT_MKSPEC -like "win32-msvc*") -and (Get-Command c++ -ErrorAction SilentlyContinue)) {
    c++ "@$response_file" $lflags
} Else {
    Write-Error "error: no suitable compiler found in PATH"
    Exit 1
}

$new_path = $env:PATH + "$([IO.Path]::PathSeparator)$QT_INSTALL_BINS"
[Environment]::SetEnvironmentVariable("PATH", $new_path)

# Build the real qbs
& $outexe build -f qbs.qbs -d "$tmpdir" `
    "qbs.buildVariant:$QBS_BUILD_VARIANT" "qbs.installRoot:$QBS_INSTALL_ROOT" `
    "Qt.core.prefixPath:$QTDIR"

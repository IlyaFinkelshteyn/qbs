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

var _windowsAbsolutePathPattern = new RegExp("^[a-z,A-Z]:[/,\\\\]");
var _removeDoubleSlashesPattern = new RegExp("/{2,}", "g");

export function path(fp: string) {
    if (fp === '/')
        return fp;

    // Yes, this will be wrong for "clever" unix users calling their directory 'c:'. Boohoo.
    if (fp.length === 3 && fp.slice(-2) === ":/")
        return fp;

    var last = fp.lastIndexOf('/');
    if (last < 0)
        return '.';
    return fp.slice(0, last);
}

export function fileName(fph: string) {
    var fp = fph.toString();
    var last = fp.lastIndexOf('/');
    if (last < 0)
        return fp;
    return fp.slice(last + 1);
}

export function baseName(fph: string) {
    var fn = fileName(fph);
    return fn.split('.')[0];
}

export function completeBaseName(fph: string) {
    var fn = fileName(fph);
    var last = fn.lastIndexOf('.');
    if (last < 0)
        return fn;
    else
        return fn.slice(0, last);
}

export function relativePath(base: string, rel: string) {
    var basel = base.split('/');
    var rell = rel.split('/');
    var i: number;
    for (i = basel.length; i-- >= 0;) {
        if (basel[i] === '.' || basel[i] === '')
            basel.splice(i, 1);
    }
    for (i = rell.length; i-- >= 0;) {
        if (rell[i] === '.' || rell[i] === '')
            rell.splice(i, 1);
    }

    i = 0;
    while (i < basel.length && i < rell.length && basel[i] === rell[i])
        i++;

    var j = i;
    var r: string[] = [];

    for (; i < basel.length; i++)
        r.push("..");

    for (; j < rell.length; j++)
        r.push(rell[j]);

    return r.join('/');
}

export function isAbsolutePath(path: string) {
    if (!path)
        return false;
    return (path.charAt(0) === '/' || _windowsAbsolutePathPattern.test(path));
}

export function toWindowsSeparators(str: string) {
    return str.toString().replace(/\//g, '\\');
}

export function fromWindowsSeparators(str: string) {
    return str.toString().replace(/\\/g, '/');
}

export function toNativeSeparators(str: string, os: string[]): string {
    return os.contains("windows") ? toWindowsSeparators(str) : str;
}

export function fromNativeSeparators(str: string, os: string[]): string {
    return os.contains("windows") ? fromWindowsSeparators(str) : str;
}

export function joinPaths(...paths: string[]) {
    function pathFilter(path: string) {
        return path && typeof path === "string";
    }

    var joinedPaths = paths.filter(pathFilter).join('/');

    return joinedPaths.replace(_removeDoubleSlashesPattern, '/');
}

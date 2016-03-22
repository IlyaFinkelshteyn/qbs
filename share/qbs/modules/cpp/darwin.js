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

var ModUtils = loadExtension("qbs.ModUtils");

function prepareLipo(project, product, inputs, outputs, input, output) {
    var commands = [], cmd, allInputs = [].concat.apply([], Object.keys(inputs).map(function (tag) {
        return inputs[tag];
    }));
    if (allInputs.length > 1) {
        cmd = new Command(ModUtils.moduleProperty(product, "lipoPath"),
                          ["-create", "-output", outputs.primary[0].filePath].concat(
                              allInputs.map(function (f) { return f.filePath })));
        cmd.description = "lipo " + outputs.primary[0].fileName;
        cmd.highlight = "linker";
    } else {
        cmd = new Command("cp", [allInputs[0].filePath, outputs.primary[0].filePath]);
        cmd.silent = true;
    }
    commands.push(cmd);

    var dsymPath = outputs.debuginfo[0].filePath;
    if (outputs.debuginfo_bundle && outputs.debuginfo_bundle[0])
        dsymPath = outputs.debuginfo_bundle[0].filePath;
    var flags = ModUtils.moduleProperty(product, "dsymutilFlags") || [];
    cmd = new Command(ModUtils.moduleProperty(product, "dsymutilPath"), flags.concat([
        "-o", dsymPath, outputs.primary[0].filePath
    ]));
    cmd.description = "generating dSYM for " + product.name;
    commands.push(cmd);

    cmd = new Command(ModUtils.moduleProperty(product, "stripPath"),
                      ["-S", outputs.primary[0].filePath]);
    cmd.silent = true;
    commands.push(cmd);


    return commands;
}

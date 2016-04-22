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
import qbs.File
import qbs.FileInfo
import "codesign.js" as CodeSign

Module {
    condition: false

    property bool enableCodeSigning: false

    property string codesignName
    property string codesignPath: codesignName
    property stringList codesignFlags

    Rule {
        inputs: ["unsigned_application"]

        outputFileTags: ["application", "codesign.signed_artifact"]
        outputArtifacts: CodeSign.outputArtifacts(product, input, "application")

        prepare: CodeSign.prepareSign.apply(this, arguments)
    }

    Rule {
        inputs: ["unsigned_dynamiclibrary"]

        outputFileTags: ["dynamiclibrary", "codesign.signed_artifact"]
        outputArtifacts: CodeSign.outputArtifacts(product, input, "dynamiclibrary")

        prepare: CodeSign.prepareSign.apply(this, arguments)
    }

    Rule {
        inputs: ["unsigned_loadablemodule"]

        outputFileTags: ["loadablemodule", "codesign.signed_artifact"]
        outputArtifacts: CodeSign.outputArtifacts(product, input, "loadablemodule")

        prepare: CodeSign.prepareSign.apply(this, arguments)
    }

    Rule {
        inputs: ["unsigned_bundle", "codesign.xcent"]
        multiplex: true

        outputFileTags: ["bundle", "codesign.signed_artifact", "bundle.codesignature"]
        outputArtifacts: CodeSign.outputArtifacts(product, inputs.unsigned_bundle[0], "bundle")

        prepare: CodeSign.prepareSign.apply(this, arguments)
    }
}

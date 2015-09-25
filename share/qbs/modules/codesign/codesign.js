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

var File = loadExtension("qbs.File");
var PropertyList = loadExtension("qbs.PropertyList");

function findSigningIdentities(searchString, team) {
    if (!searchString)
        return {};
    var identities = qbs.internal.signingIdentities();
    var matchedIdentities = {};
    for (var key in identities) {
        var identity = identities[key];
        if (team && ![identity.subjectName.O, identity.subjectName.OU].contains(team))
            continue;
        if (searchString === key || identity.subjectName.CN.startsWith(searchString))
            matchedIdentities[key] = identity;
    }
    return matchedIdentities;
}

function humanReadableIdentitySummary(identities) {
    return "\n\t" + Object.keys(identities).map(function (key) {
        return identities[key].subjectName.CN
                + " in team "
                + identities[key].subjectName.O
                + " (" + identities[key].subjectName.OU + ")";
    }).join("\n\t");
}

/**
  * Returns the best provisioning profile for code signing a binary with the given parameters.
  * Ideally, this should behave identically as Xcode but the algorithm is not documented
  * \l{https://developer.apple.com/library/ios/qa/qa1814/_index.html}{Automatic Provisioning}
  */
function findBestProvisioningProfile(product, files) {
    var actualSigningIdentity = product.moduleProperty("codesign", "_actualSigningIdentity") || {};
    var teamIdentifier = product.moduleProperty("codesign", "teamIdentifier");
    var bundleIdentifier = product.moduleProperty("bundle", "identifier");
    var targetOS = product.moduleProperty("qbs", "targetOS");
    var buildVariant = product.moduleProperty("qbs", "buildVariant");
    var query = product.moduleProperty("codesign", "provisioningProfile");

    // Read all provisioning profiles on disk into plist objects in memory
    var profiles = files.map(function(filePath) {
        var plist = new PropertyList();
        try {
            plist.readFromData(qbs.internal.smimeMessageContent(filePath));
            return {
                data: plist.toObject(),
                filePath: filePath
            };
        } finally {
            plist.clear();
        }
    });

    // Do a simple search by matching UUID or Name
    if (query) {
        for (var i = 0; i < profiles.length; ++i) {
            var obj = profiles[i];
            if (obj.data && (obj.data.UUID === query || obj.data.Name === query))
                return obj;
        }

        // If we asked for a specific provisioning profile, don't select one automatically
        return undefined;
    }

    // Provisioning profiles are not normally used with ad-hoc code signing or non-apps
    // We do these checks down here only for the automatic selection but not above because
    // if the user explicitly selects a provisioning profile it should be used no matter what
    if (actualSigningIdentity.SHA1 === "-" || !product.type.contains("application"))
        return undefined;

    // Filter out any provisioning profiles we know to be unsuitable from the start
    profiles = profiles.filter(function (profile) {
        var data = profile.data;

        if (actualSigningIdentity.subjectName) {
            var certCommonNames = (data["DeveloperCertificates"] || []).map(function (cert) {
                return qbs.internal.certificateCommonName(cert);
            });
            if (!certCommonNames.contains(actualSigningIdentity.subjectName.CN)) {
                console.log("Skipping provisioning profile with no matching certificate names for '"
                            + actualSigningIdentity.subjectName.CN
                            + "' (found " + certCommonNames.join(", ") + "): "
                            + profile.filePath);
                return false;
            }
        }

        if ((data["Platform"] || []).contains("OSX") && !targetOS.contains("osx")) {
            console.log("Skipping OS X provisioning profile: " + profile.filePath);
            return false;
        }

        if (teamIdentifier
                && !data["TeamIdentifier"].contains(teamIdentifier)
                && data["TeamName"] !== teamIdentifier) {
            console.log("Skipping provisioning profile for team " + data["TeamIdentifier"]
                        + " (" + data["TeamName"] + ") (current team " + teamIdentifier + ")"
                        + ": " + profile.filePath);
            return false;
        }

        if (Date.parse(data["ExpirationDate"]) <= Date.now()) {
            console.log("Skipping expired provisioning profile: " + profile.filePath);
            return false;
        }

        // Filter development vs distribution profiles;
        // though the certificate common names check should have been sufficient
        var isDebug = buildVariant === "debug";
        if (data["Entitlements"]["get-task-allow"] !== isDebug) {
            console.log("Skipping provisioning profile for wrong debug mode: " + profile.filePath);
            return false;
        }

        var prefix = data["ApplicationIdentifierPrefix"];
        var fullAppId = data["Entitlements"]["application-identifier"];
        if ([prefix, bundleIdentifier].join(".") !== fullAppId
                && [prefix, "*"].join(".") !== fullAppId) {
            console.log("Skipping provisioning profile not matching full ("
                        + [prefix, bundleIdentifier].join(".") + ") or wildcard ("
                        + [prefix, "*"].join(".") + ") app ID (found " + fullAppId + "): "
                        + profile.filePath);
            return false;
        }

        return true;
    });

    // Sort by expiration date - sooner expiration dates come last
    profiles.sort(function(profileA, profileB) {
        var expA = Date.parse(profileA.data["ExpirationDate"]);
        var expB = Date.parse(profileB.data["ExpirationDate"]);
        if (expA < expB)
            return -1;
        if (expA > expB)
            return 1;
        return 0;
    });

    // Sort by application identifier - wildcard profiles come last
    profiles.sort(function(profileA, profileB) {
        var idA = profileA.data["Entitlements"]["application-identifier"];
        var idB = profileB.data["Entitlements"]["application-identifier"];
        if (!idA.endsWith(".*") && idB.endsWith(".*"))
            return -1;
        if (idA.endsWith(".*") && !idB.endsWith(".*"))
            return 1;
        return 0;
    });

    if (profiles.length) {
        console.log("Automatic provisioning using profile "
                    + profiles[0].data.UUID
                    + " ("
                    + profiles[0].data.TeamName
                    + " - "
                    + profiles[0].data.Name
                    + ") in product "
                    + product.name);
        return profiles[0];
    }
}

function prepareSign(project, product, inputs, outputs, input, output) {
    var outputFileName = outputs["codesign.signed_artifact"][0].fileName;
    var outputFilePath = outputs["codesign.signed_artifact"][0].filePath;
    var isBundle = outputs["codesign.signed_artifact"][0].fileTags.contains("bundle");
    var isProductBundle = product.moduleProperty("bundle", "isBundle");

    // If the product is a bundle, just sign the bundle
    // instead of signing the bundle and executable separately
    var shouldSignArtifact = !isProductBundle || isBundle;

    var cmd, cmds = [];
    if (!isBundle) {
        cmd = new JavaScriptCommand();
        cmd.src = input.filePath;
        cmd.dst = outputFilePath;
        cmd.silent = true;
        cmd.sourceCode = function() {
            File.remove(dst);
            File.copy(src, dst);
        };
        cmds.push(cmd);
    }

    var enableCodeSigning = product.moduleProperty("codesign", "enableCodeSigning");
    if (enableCodeSigning && shouldSignArtifact) {
        var actualSigningIdentity = product.moduleProperty("codesign", "_actualSigningIdentity");
        if (!actualSigningIdentity) {
            throw "No codesigning identities (i.e. certificate and private key pairs) matching “"
                    + product.moduleProperty("codesign", "signingIdentity") + "” were found.";
        }

        // If this is a framework, we need to sign its versioned directory
        var subpath = "";
        if (isBundle) {
            var frameworkVersion = product.moduleProperty("bundle", "frameworkVersion");
            if (frameworkVersion) {
                subpath = product.moduleProperty("bundle", "contentsFolderPath");
                subpath = subpath.substring(product.moduleProperty("bundle", "bundleName").length);
            }
        }

        var args = product.moduleProperty("codesign", "codesignFlags") || [];
        args.push("--force");
        args.push("--sign", actualSigningIdentity.SHA1);

        // If signingTimestamp is undefined, do not specify the flag at all -
        // this uses the system-specific default behavior
        var signingTimestamp = product.moduleProperty("codesign", "signingTimestamp");
        if (signingTimestamp !== undefined) {
            // If signingTimestamp is an empty string, specify the flag but do
            // not specify a value - this uses a default Apple-provided server
            var flag = "--timestamp";
            if (signingTimestamp)
                flag += "=" + signingTimestamp;
            args.push(flag);
        }

        for (var j in inputs["codesign.xcent"]) {
            args.push("--entitlements", inputs["codesign.xcent"][j].filePath);
            break; // there should only be one
        }
        args.push(outputFilePath + subpath);
        cmd = new Command(product.moduleProperty("codesign", "codesignPath"), args);
        cmd.description = "codesign " + outputFileName
                + " (" + actualSigningIdentity.subjectName.CN + ")";
        cmd.outputFilePath = outputFilePath;
        cmd.stderrFilterFunction = function(stderr) {
            return stderr.replace(outputFilePath + ": replacing existing signature\n", "");
        };
        cmds.push(cmd);
    }

    if (isBundle) {
        cmd = new Command("touch", ["-c", outputFilePath]);
        cmd.silent = true;
        cmds.push(cmd);
    }

    return cmds;
}

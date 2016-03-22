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

Product {
    type: {
        if (isForAndroid && !consoleApplication)
            return ["dynamiclibrary", "android.nativelibrary"];
        return ["application"];
    }

    property bool isForAndroid: qbs.targetOS.contains("android")

    Depends { name: "Android.ndk"; condition: isForAndroid }
    Depends { name: "bundle" }
    Depends { name: "cpp" }

    // Top-level instance depends on all architecture-specific instances
    property stringList architectures: qbs.architectures
    property stringList architectureProfiles: (architectures || []).map(function(arch) { return project.profile + '-' + arch; })
    property bool hasAggregateProfile: qbs.targetOS.contains("darwin")
    Depends {
        id: aggregateProfileDependency
        name: product.name
        condition: profile === project.profile && hasAggregateProfile
        profiles: architectureProfiles
    }

    // If we're building for more than one architecture, multiplex, or if we use a top-level profile
    // in which case we always need to multiplex, even for more than one architecture
    profiles: architectures.length > 1 || hasAggregateProfile
        ? (hasAggregateProfile ? [project.profile] : []).concat(architectureProfiles)
        : [project.profile]
}

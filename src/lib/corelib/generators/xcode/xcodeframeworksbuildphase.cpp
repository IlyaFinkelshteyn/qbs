/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qbs.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "xcodeframeworksbuildphase.h"
#include "pbx/pbxbuildfile.h"
#include "pbx/pbxbuildphase.h"
#include "pbx/pbxfilereference.h"
#include "pbx/pbxgroup.h"
#include "pbx/pbxproject.h"
#include "pbx/pbxtarget.h"
#include <tools/qbsassert.h>

namespace qbs {

class XcodeFrameworksBuildPhasePrivate {
public:
    PBXProject *xcodeProject;
    PBXTarget *xcodeTarget;
};

XcodeFrameworksBuildPhase::XcodeFrameworksBuildPhase(PBXProject *xcodeProject,
                                                     PBXTarget *xcodeTarget)
    : d(QSharedPointer<XcodeFrameworksBuildPhasePrivate>::create())
{
    d->xcodeProject = xcodeProject;
    d->xcodeTarget = xcodeTarget;
}

void XcodeFrameworksBuildPhase::addLinkedLibrary(const QString &sysroot,
                                                 const QString &absoluteLibraryPath,
                                                 bool weak)
{
    // add 1 for the path separator
    const auto sysrootSize = !sysroot.isEmpty() ? sysroot.size() + 1 : 0;
    const auto sysrootRelativeLibraryPath = absoluteLibraryPath.mid(sysrootSize);

    // Check for an existing reference (could happen if a library is specified multiple times)
    PBXFileReference *existingReference = Q_NULLPTR;
    for (const auto &ref : d->xcodeProject->frameworksRefGroup()->children()) {
        if (const auto &fileRef = dynamic_cast<PBXFileReference *>(ref)) {
            if ((fileRef->sourceTree() == PBX::SdkRoot && fileRef->path() == sysrootRelativeLibraryPath) ||
                    (fileRef->sourceTree() == PBX::Absolute && fileRef->path() == absoluteLibraryPath)) {
                existingReference = fileRef;
                break;
            }
        }
    }

    QList<PBXFileReference *> refs;

    // Create a new file reference since one wasn't already found in the Frameworks group
    if (!existingReference) {
        refs = d->xcodeProject->frameworksRefGroup()->addFiles(QStringList() << absoluteLibraryPath);
        for (const auto &libraryRef : refs) {
            if (!sysroot.isEmpty() && libraryRef->path().startsWith(sysroot)) {
                libraryRef->setPath(sysrootRelativeLibraryPath);
                libraryRef->setSourceTree(PBX::SdkRoot);
            }
        }
    } else {
        refs.append(existingReference);
    }

    // Add the library to the Frameworks build phase
    for (const auto &libraryRef : refs) {
        if (const auto &libraryBuildRef = d->xcodeTarget->defaultFrameworksBuildPhase()->addReference(libraryRef))
            libraryBuildRef->setWeakLinkedLibrary(weak);
    }
}

void XcodeFrameworksBuildPhase::addBrokenLinkedLibrary(const QString &libraryName,
                                                       bool weak)
{
    const auto theLibraryFileName = libraryName;
    for (auto libraryRef : d->xcodeProject->frameworksRefGroup()->addFiles(QStringList() << theLibraryFileName)) {
        QBS_CHECK(!libraryRef->path().isEmpty());
        if (const auto &libraryBuildRef = d->xcodeTarget->defaultFrameworksBuildPhase()->addReference(libraryRef))
            libraryBuildRef->setWeakLinkedLibrary(weak);
    }
}

void XcodeFrameworksBuildPhase::addLinkables(const QStringList &libraryNames,
                                             QStringList &searchPaths,
                                             const QString &sysroot, const QString &prefix,
                                             const QString &suffix, bool weak)
{
    for (const auto &libraryName : libraryNames) {
        // Whether a full path equivalent for the library was found
        bool foundFullPath = false;

        for (const auto &searchPath : searchPaths) {
            const auto candiateLibraryPath = QFileInfo(searchPath + QStringLiteral("/") + prefix + libraryName + suffix).canonicalFilePath();
            if (QFileInfo(candiateLibraryPath).exists()) {
                addLinkedLibrary(sysroot, candiateLibraryPath, weak);
                foundFullPath = true;
                break;
            }
        }

        // Not a normal case, and will cause a build failure, but is better than silently ignoring it...
        if (!foundFullPath)
            addBrokenLinkedLibrary(prefix + libraryName + suffix, weak);
    }
}

void XcodeFrameworksBuildPhase::addLinkables(const GeneratableProductData &product)
{
    // Note: this essentially aggregates dependencies between all configurations.
    // For PBXLegacyTargets (our primary use case), it won't matter, but a smarter approach might
    // be to aggregate ONLY dependencies which are common to ALL configurations here,
    // and use raw linker flags for the differing ones.
    product.forEach([&](const QString &, const qbs::ProductData &productData) {
        QString sysroot = productData.moduleProperties().getModuleProperty(QStringLiteral("cpp"), QStringLiteral("sysroot")).toString();

        QStringList frameworkPaths;
        frameworkPaths += productData.moduleProperties().getModulePropertiesAsStringList(QStringLiteral("cpp"), QStringLiteral("frameworkPaths"));
        frameworkPaths += productData.moduleProperties().getModulePropertiesAsStringList(QStringLiteral("cpp"), QStringLiteral("systemFrameworkPaths"));
        frameworkPaths += sysroot + QStringLiteral("/Library/Frameworks");
        frameworkPaths += sysroot + QStringLiteral("/System/Library/Frameworks");
        frameworkPaths += QStringLiteral("/Library/Frameworks");
        frameworkPaths += QStringLiteral("/System/Library/Frameworks");
        const auto frameworkNames = productData.moduleProperties().getModulePropertiesAsStringList(QStringLiteral("cpp"), QStringLiteral("frameworks"));
        const auto weakFrameworkNames = productData.moduleProperties().getModulePropertiesAsStringList(QStringLiteral("cpp"), QStringLiteral("weakFrameworks"));

        QStringList libraryPaths;
        libraryPaths += productData.moduleProperties().getModulePropertiesAsStringList(QStringLiteral("cpp"), QStringLiteral("libraryPaths"));
        libraryPaths += productData.moduleProperties().getModulePropertiesAsStringList(QStringLiteral("cpp"), QStringLiteral("systemLibraryPaths"));
        libraryPaths += sysroot + QStringLiteral("/usr/lib");
        libraryPaths += QStringLiteral("/usr/lib");
        auto dynamicLibraries = productData.moduleProperties().getModulePropertiesAsStringList(QStringLiteral("cpp"), QStringLiteral("dynamicLibraries"));
        auto staticLibraries = productData.moduleProperties().getModulePropertiesAsStringList(QStringLiteral("cpp"), QStringLiteral("staticLibraries"));

        // A framework can't be strongly AND weakly linked...
        if (frameworkNames.toSet().intersects(weakFrameworkNames.toSet()))
            throw ErrorInfo(QStringLiteral("One or more frameworks appears in both cpp.frameworks and cpp.weakFrameworks"));

        const auto allLibraries = QStringList() << dynamicLibraries << staticLibraries;
        for (const auto &library : allLibraries) {
            const QFileInfo fi(library);
            if (fi.isAbsolute()) {
                QString suffix;
                if (dynamicLibraries.removeOne(library))
                    suffix = QStringLiteral(".dylib");
                if (staticLibraries.removeOne(library))
                    suffix = QStringLiteral(".a");

                if (fi.exists())
                    addLinkedLibrary(sysroot, library);
                else
                    addBrokenLinkedLibrary(QStringLiteral("lib") + library + suffix);
            }
        }

        addLinkables(frameworkNames, frameworkPaths, sysroot, QString(), QStringLiteral(".framework"));
        addLinkables(weakFrameworkNames, frameworkPaths, sysroot, QString(), QStringLiteral(".framework"), true);
        addLinkables(dynamicLibraries, libraryPaths, sysroot, QStringLiteral("lib"), QStringLiteral(".dylib"));
        addLinkables(staticLibraries, libraryPaths, sysroot, QStringLiteral("lib"), QStringLiteral(".a"));
    });
}

} // namespace qbs

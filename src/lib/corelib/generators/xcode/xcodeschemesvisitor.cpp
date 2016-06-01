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

#include "xcodeschemesvisitor.h"
#include "pbx/pbxtarget.h"
#include "pbx/pbxtargetdependency.h"
#include "xcscheme/xcscheme.h"
#include <logging/translator.h>

namespace qbs {

using namespace Internal;

void XcodeSchemesVisitor::visitProduct(const GeneratableProject &project,
                                       const GeneratableProjectData &projectData,
                                       const GeneratableProductData &productData)
{
    PBXTarget *qbsXcodeTarget = xcodeProject->targetNamed(productData.name());
    if (!qbsXcodeTarget)
        throw new qbs::ErrorInfo(Tr::tr("could not find PBXTarget for qbs product '%1'").arg(productData.name()));

    // Set up target dependencies
    for (const QString &dependencyName : productData.dependencies()) {
        PBXTargetDependency *dependentXcodeTarget = new PBXTargetDependency(xcodeProject.data());
        dependentXcodeTarget->setTarget(xcodeProject->targetNamed(dependencyName));
        if (!dependentXcodeTarget->target())
            throw new qbs::ErrorInfo(Tr::tr("could not find PBXTarget for qbs product '%1'").arg(dependencyName));

        qbsXcodeTarget->targetDependencies().append(dependentXcodeTarget);
    }

    // Set up a scheme for the target
    QDir baseBuildDirectory = project.baseBuildDirectory();
    const QString xcodeWrapperName = project.name() + QStringLiteral(".xcodeproj");
    if (!baseBuildDirectory.cd(xcodeWrapperName))
        if (!baseBuildDirectory.mkdir(xcodeWrapperName)
                || !baseBuildDirectory.cd(xcodeWrapperName))
            throw ErrorInfo(Tr::tr("failed to make Xcode project wrapper directory"));

    static const QString xcshareddata = QStringLiteral("xcshareddata");
    static const QString xcschemes = QStringLiteral("xcschemes");
    QDir schemesDirectory(baseBuildDirectory);
    if (!schemesDirectory.cd(xcshareddata)) {
        if (!schemesDirectory.mkdir(xcshareddata) || !schemesDirectory.cd(xcshareddata)) {
            throw ErrorInfo(Tr::tr("failed to make Xcode project shared data directory"));
        }
    }
    if (!schemesDirectory.cd(xcschemes)) {
        if (!schemesDirectory.mkdir(xcschemes) || !schemesDirectory.cd(xcschemes)) {
            throw ErrorInfo(Tr::tr("failed to make Xcode project shared schemes directory"));
        }
    }

    const QString schemeFilePath(schemesDirectory.absoluteFilePath(QString(QStringLiteral("%1.xcscheme")).arg(qbsXcodeTarget->name())));
    XCScheme xcscheme;
    xcscheme.setParallelizeBuildables(false); // this must be turned off because it conflicts with qbs build directory locking
    xcscheme.setProject(xcodeProject.data());
    xcscheme.setTarget(qbsXcodeTarget);
    //xcscheme.setRunEnvironment(project.getRunEnvironment(productData, project.installOptions, QProcessEnvironment(), nullptr).runEnvironment(nullptr));
    // SCHEMES ARE PER CONFIG SO RUN ENV ONLY SET ONCE!
    // TODO: use debug by default or use that of first config on cmd line?

//    if (dynamic_cast<PBXLegacyTarget *>(qbsXcodeTarget))
//        xcscheme.setCustomExecutableFilePath(runnableOrPrimaryTargetArtifactForProduct(productData).installData().installFilePath());

    if (!xcscheme.serialize(schemeFilePath))
        throw ErrorInfo(Tr::tr("Failed to generate scheme %1").arg(schemeFilePath));
}

} // namespace qbs

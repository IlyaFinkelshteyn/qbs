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

#include "pbxprojectwriter.h"
#include "opensteppropertylist.h"

#include "pbx/pbxbuildfile.h"
#include "pbx/pbxbuildphase.h"
#include "pbx/pbxbuildrule.h"
#include "pbx/pbxcontaineritemproxy.h"
#include "pbx/pbxgroup.h"
#include "pbx/pbxnativetarget.h"
#include "pbx/pbxproject.h"
#include "pbx/pbxtargetdependency.h"
#include "pbx/xcbuildconfiguration.h"
#include "pbx/xcconfigurationlist.h"

namespace qbs {

class PBXProjectWriterPrivate
{
public:
    QIODevice *device;
};

PBXProjectWriter::PBXProjectWriter(QIODevice *device)
    : d(new PBXProjectWriterPrivate)
{
    d->device = device;
}

PBXProjectWriter::~PBXProjectWriter()
{
    delete d;
}

static void writeGroup(PBXObjectMap &objects, PBXGroup *group)
{
    objects.insert(group->identifier(), QVariant::fromValue(group->toMap()));
    for (PBXReference *child : group->children()) {
        if (PBXGroup *group = dynamic_cast<PBXGroup *>(child))
            writeGroup(objects, group);
        else
            objects.insert(child->identifier(), QVariant::fromValue(child->toMap()));
    }
}

bool PBXProjectWriter::write(const PBXProject *project)
{
    PBXObjectMap objects;
    objects.insert(project->identifier(), QVariant::fromValue(project->toMap()));

    // Add the project's root group (i.e. the root node in the file tree which contains everything)
    writeGroup(objects, project->rootGroup());

    // Add the project's build configuration list and build configurations to the object map
    objects.insert(project->buildConfigurationList()->identifier(), QVariant::fromValue(project->buildConfigurationList()->toMap()));
    for (XCBuildConfiguration *configuration : project->buildConfigurationList()->buildConfigurations())
        objects.insert(configuration->identifier(), QVariant::fromValue(configuration->toMap()));

    // Add the build targets to the object map
    for (PBXTarget *target : project->targets()) {
        objects.insert(target->identifier(), QVariant::fromValue(target->toMap()));

        // Add the target's build configuration list and build configurations to the object map
        objects.insert(target->buildConfigurationList()->identifier(), QVariant::fromValue(target->buildConfigurationList()->toMap()));
        for (XCBuildConfiguration *configuration : target->buildConfigurationList()->buildConfigurations())
            objects.insert(configuration->identifier(), QVariant::fromValue(configuration->toMap()));

        // Add the target's dependency objects to the object map
        for (PBXTargetDependency *dependency : target->targetDependencies()) {
            objects.insert(dependency->identifier(), QVariant::fromValue(dependency->toMap()));

            if (dependency->targetProxy())
                objects.insert(dependency->targetProxy()->identifier(), QVariant::fromValue(dependency->targetProxy()->toMap()));
        }

        // Add the target's build phases...
        for (PBXBuildPhase *buildPhase : target->buildPhases()) {
            objects.insert(buildPhase->identifier(), QVariant::fromValue(buildPhase->toMap()));

            // And each of the files within
            for (PBXBuildFile *buildFile : buildPhase->buildFiles())
                objects.insert(buildFile->identifier(), QVariant::fromValue(buildFile->toMap()));
        }

        // Add the target's build rules...
        if (PBXNativeTarget *nativeTarget = dynamic_cast<PBXNativeTarget *>(target)) {
            for (PBXBuildRule *buildRule : nativeTarget->buildRules())
                objects.insert(buildRule->identifier(), QVariant::fromValue(buildRule->toMap()));
        }
    }

    // add all subobjects to objects map!

    // Build the root pbxproj object map
    PBXObjectMap map;
    map.insert(QStringLiteral("archiveVersion"), QStringLiteral("1"));
    map.insert(QStringLiteral("classes"), QVariantMap());
    map.insert(QStringLiteral("objectVersion"), QStringLiteral("46"));
    map.insert(QStringLiteral("objects"), QVariant::fromValue(objects));
    map.insert(QStringLiteral("rootObject"), QVariant::fromValue(project->identifier()));

    if (d->device->write(QByteArray("// !$*UTF8*$!\n")) == -1)
        return false;

    if (d->device->write(OpenStepPropertyList::toString(map).toUtf8()) == -1)
        return false;

    return true;
}

} // namespace qbs

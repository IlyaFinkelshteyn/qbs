/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2015 Jake Petroules.
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Build Suite.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "opensteppropertylist.h"
#include "pbxbuildphase.h"
#include "pbxbuildfile.h"
#include "pbxcontaineritemproxy.h"
#include "pbxgroup.h"
#include "pbxproject.h"
#include "pbxtarget.h"
#include "pbxtargetdependency.h"
#include "xcbuildconfiguration.h"
#include "xcconfigurationlist.h"
#include <QFile>
#include <QFileInfo>
#include <QVariant>

static const QString kClassPrefixAttribute = QLatin1String("CLASSPREFIX");
static const QString kLastUpgradeCheckAttribute = QLatin1String("LastUpgradeCheck");
static const QString kOrganizationNameAttribute = QLatin1String("ORGANIZATIONNAME");
static const QString kTargetAttributesAttribute = QLatin1String("TargetAttributes");

class PBXProjectPrivate
{
public:
    QString name;
    QString projectAbsolutePath;
    QString projectDirPath;
    PBXGroup *frameworksRefGroup;
    PBXGroup *productRefGroup;
    QList<PBXTarget *> targets;
    XCConfigurationList *configurationList;
    QVariantMap attributes;
};

PBXProject::PBXProject(const QString &projectAbsolutePath, QObject *parent) :
    PBXContainer(parent), d(new PBXProjectPrivate)
{
    d->projectAbsolutePath = projectAbsolutePath;
    d->configurationList = new XCConfigurationList(this);

    d->frameworksRefGroup = new PBXGroup(rootGroup());
    d->frameworksRefGroup->setName(QLatin1String("Frameworks"));

    d->productRefGroup = new PBXGroup(rootGroup());
    d->productRefGroup->setName(QLatin1String("Products"));
    d->productRefGroup->setSourceTree(PBX::Group);
}

PBXProject::~PBXProject()
{
    delete d;
}

QString PBXProject::isa() const
{
    return QLatin1String("PBXProject");
}

PBXObjectMap PBXProject::toMap() const
{
    // HACK: Make sure "Frameworks", "Products" groups are always at the bottom
    rootGroup()->removeItem(d->frameworksRefGroup);
    rootGroup()->removeItem(d->productRefGroup);
    rootGroup()->addItem(d->frameworksRefGroup);
    rootGroup()->addItem(d->productRefGroup);

    PBXObjectMap self = PBXObject::toMap();
    self.insert(QLatin1String("attributes"), d->attributes);
    self.insert(QLatin1String("buildConfigurationList"), QVariant::fromValue(buildConfigurationList()->identifier()));
    self.insert(QLatin1String("compatibilityVersion"), QLatin1String("Xcode 3.2"));
    self.insert(QLatin1String("developmentRegion"), QLatin1String("English"));
    self.insert(QLatin1String("hasScannedForEncodings"), QLatin1String("1"));
    self.insert(QLatin1String("knownRegions"), QStringList() << QLatin1String("en"));
    self.insert(QLatin1String("mainGroup"), QVariant::fromValue(rootGroup()->identifier()));
    self.insert(QLatin1String("productRefGroup"), QVariant::fromValue(productRefGroup()->identifier()));
    self.insert(QLatin1String("projectDirPath"), d->projectDirPath);
    //self.insert(QLatin1String("projectReferences"), QVariantList()); // array of map { ProductGroup:, ProjectRef: }?
    self.insert(QLatin1String("projectRoot"), QLatin1String(""));

    QVariantList targetIdentifiers;
    foreach (PBXTarget *target, targets())
        targetIdentifiers += QVariant::fromValue(target->identifier());
    self.insert(QLatin1String("targets"), targetIdentifiers);

    return self;
}

QString PBXProject::comment() const
{
    return QLatin1String("Project object");
}

QByteArray PBXProject::hashData() const
{
    QByteArray data = PBXObject::hashData();
    data.append(d->name);
    return data;
}

PBXGroup *PBXProject::productRefGroup() const
{
    return d->productRefGroup;
}

PBXGroup *PBXProject::frameworksRefGroup() const
{
    return d->frameworksRefGroup;
}

QList<PBXTarget *> PBXProject::targets() const
{
    return d->targets;
}

PBXTarget *PBXProject::targetNamed(const QString &targetName) const
{
    foreach (PBXTarget *target, d->targets)
    {
        if (target->name() == targetName)
            return target;
    }

    return NULL;
}

void PBXProject::addTarget(PBXTarget *target)
{
    d->targets.append(target);
}

QString PBXProject::name() const
{
    return d->name;
}

void PBXProject::setName(const QString &name)
{
    d->name = name;
}

QVariantMap PBXProject::attributes() const
{
    return d->attributes;
}

void PBXProject::setAttributes(const QVariantMap &attributes)
{
    d->attributes = attributes;
}

QVariant PBXProject::attribute(const QString &name) const
{
    return d->attributes[name];
}

void PBXProject::setAttribute(const QString &name, const QVariant &value)
{
    d->attributes[name] = value;
}

void PBXProject::removeAttribute(const QString &name)
{
    d->attributes.remove(name);
}

PBXProject::Version PBXProject::lastUpgradeCheck() const
{
    const QString s = d->attributes[kLastUpgradeCheckAttribute].toString().mid(0, 3);
    Version v;
    v.major = s.left(2).toInt();
    v.minor = s.right(1).toInt();
    return v;
}

void PBXProject::setLastUpgradeCheck(const PBXProject::Version &version)
{
    const int major = qBound(0, version.major, 99);
    const int minor = qBound(0, version.minor, 9);
    d->attributes[kLastUpgradeCheckAttribute] =
            QString::number(major).rightJustified(2, QLatin1Char('0')) +
            QString::number(minor).leftJustified(2, QLatin1Char('0'));
}

QString PBXProject::classPrefix() const
{
    return d->attributes[kClassPrefixAttribute].toString();
}

void PBXProject::setClassPrefix(const QString &classPrefix)
{
    d->attributes[kClassPrefixAttribute] = classPrefix;
}

QString PBXProject::organizationName() const
{
    return d->attributes[kOrganizationNameAttribute].toString();
}

void PBXProject::setOrganizationName(const QString &organizationName)
{
    d->attributes[kOrganizationNameAttribute] = organizationName;
}

QString PBXProject::projectWrapperPath() const
{
    return QFileInfo(d->projectAbsolutePath).absolutePath();
}

QString PBXProject::projectDirPath() const
{
    return d->projectDirPath;
}

void PBXProject::setProjectDirPath(const QString &projectDirPath)
{
    d->projectDirPath = projectDirPath;
}

void PBXProject::setBuildSetting(const QString &key, const QVariant &value)
{
    foreach (XCBuildConfiguration *configuration, d->configurationList->buildConfigurations())
        configuration->setBuildSetting(key, value);
}

XCConfigurationList *PBXProject::buildConfigurationList() const
{
    return d->configurationList;
}

static void writeGroup(PBXObjectMap &objects, PBXGroup *group)
{
    objects.insert(group->identifier(), QVariant::fromValue(group->toMap()));
    foreach (PBXReference *child, group->children())
    {
        if (PBXGroup *group = dynamic_cast<PBXGroup *>(child))
            writeGroup(objects, group);
        else
            objects.insert(child->identifier(), QVariant::fromValue(child->toMap()));
    }
}

bool PBXProject::writeToFileSystemProjectFile(bool projectWrite, bool userWrite, bool checkNeedsRevert)
{
    PBXObjectMap objects;
    objects.insert(identifier(), QVariant::fromValue(toMap()));

    // Add the project's root group (i.e. the root node in the file tree which contains everything)
    writeGroup(objects, rootGroup());

    // Add the project's build configuration list and build configurations to the object map
    objects.insert(d->configurationList->identifier(), QVariant::fromValue(d->configurationList->toMap()));
    foreach (XCBuildConfiguration *configuration, d->configurationList->buildConfigurations())
        objects.insert(configuration->identifier(), QVariant::fromValue(configuration->toMap()));

    // Add the build targets to the object map
    foreach (PBXTarget *target, targets())
    {
        objects.insert(target->identifier(), QVariant::fromValue(target->toMap()));

        // Add the target's build configuration list and build configurations to the object map
        objects.insert(target->buildConfigurationList()->identifier(), QVariant::fromValue(target->buildConfigurationList()->toMap()));
        foreach (XCBuildConfiguration *configuration, target->buildConfigurationList()->buildConfigurations())
            objects.insert(configuration->identifier(), QVariant::fromValue(configuration->toMap()));

        // Add the target's dependency objects to the object map
        foreach (PBXTargetDependency *dependency, target->targetDependencies()) {
            objects.insert(dependency->identifier(), QVariant::fromValue(dependency->toMap()));

            if (dependency->targetProxy()) {
                objects.insert(dependency->targetProxy()->identifier(), QVariant::fromValue(dependency->targetProxy()->toMap()));
            }
        }

        // Add the target's build phases...
        foreach (PBXBuildPhase *buildPhase, target->buildPhases())
        {
            objects.insert(buildPhase->identifier(), QVariant::fromValue(buildPhase->toMap()));

            // And each of the files within
            foreach (PBXBuildFile *buildFile, buildPhase->buildFiles())
            {
                objects.insert(buildFile->identifier(), QVariant::fromValue(buildFile->toMap()));
            }
        }
    }

    // add all subobjects to objects map!

    // Build the root pbxproj object map
    PBXObjectMap map;
    map.insert(QLatin1String("archiveVersion"), QLatin1String("1"));
    map.insert(QLatin1String("classes"), QVariantMap());
    map.insert(QLatin1String("objectVersion"), QLatin1String("46"));
    map.insert(QLatin1String("objects"), QVariant::fromValue(objects));
    map.insert(QLatin1String("rootObject"), QVariant::fromValue(identifier()));

    QFile file(d->projectAbsolutePath);
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(QByteArray("// !$*UTF8*$!\n"));
        file.write(OpenStepPropertyList::toString(map).toUtf8());
    }

    return file.error() == QFile::NoError;
}

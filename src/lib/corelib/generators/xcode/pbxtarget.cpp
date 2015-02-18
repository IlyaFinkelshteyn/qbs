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

#include "pbxfilereference.h"
#include "pbxframeworksbuildphase.h"
#include "pbxproject.h"
#include "pbxsourcesbuildphase.h"
#include "pbxtarget.h"
#include "pbxtargetdependency.h"
#include "xcbuildconfiguration.h"
#include "xcconfigurationlist.h"

class PBXTargetPrivate
{
public:
    PBXTargetPrivate();
    QString name;
    QString productName;
    PBX::PBXProductType productType;
    XCConfigurationList *configurationList;
    PBXFrameworksBuildPhase *frameworksBuildPhase;
    PBXSourcesBuildPhase *sourcesBuildPhase;
    PBXFileReference *productReference;
    QList<PBXTargetDependency *> targetDependencies;
};

PBXTargetPrivate::PBXTargetPrivate()
    : name(), productType(), configurationList(), sourcesBuildPhase(), productReference(), targetDependencies()
{
}

PBXTarget::PBXTarget(PBXProject *parent) :
    PBXObject(parent), d(new PBXTargetPrivate)
{
    d->configurationList = new XCConfigurationList(this);
    d->frameworksBuildPhase = new PBXFrameworksBuildPhase(this);
    d->sourcesBuildPhase = new PBXSourcesBuildPhase(this);
}

PBXTarget::~PBXTarget()
{
    delete d;
}

PBXObjectMap PBXTarget::toMap() const
{
    PBXObjectMap self = PBXObject::toMap();
    self.insert(QLatin1String("name"), name());
    self.insert(QLatin1String("productName"), productName());
    self.insert(QLatin1String("productType"), PBX::productTypeString(d->productType));

    self.insert(QLatin1String("buildConfigurationList"), QVariant::fromValue(buildConfigurationList()->identifier()));

    QVariantList buildPhaseReferences;
    foreach (PBXBuildPhase *buildPhase, buildPhases())
        buildPhaseReferences += QVariant::fromValue(buildPhase->identifier());
    self.insert(QLatin1String("buildPhases"), buildPhaseReferences);

    QVariantList targetDependencyReferences;
    foreach (PBXTargetDependency *dependency, d->targetDependencies)
        targetDependencyReferences += QVariant::fromValue(dependency->identifier());
    self.insert(QLatin1String("dependencies"), targetDependencyReferences);

    if (d->productReference)
        self.insert(QLatin1String("productReference"), QVariant::fromValue(d->productReference->identifier()));

    return self;
}

QString PBXTarget::name() const
{
    return d->name;
}

void PBXTarget::setName(const QString &name)
{
    d->name = name;
}

QString PBXTarget::productName() const
{
    return d->productName;
}

void PBXTarget::setProductName(const QString &productName)
{
    d->productName = productName;
}

PBX::PBXProductType PBXTarget::productType() const
{
    return d->productType;
}

void PBXTarget::setProductType(PBX::PBXProductType productType)
{
    d->productType = productType;
}

XCConfigurationList *PBXTarget::buildConfigurationList() const
{
    return d->configurationList;
}

QList<PBXTargetDependency *> &PBXTarget::targetDependencies() const
{
    return d->targetDependencies;
}

QVariant PBXTarget::buildSetting(const QString &key, const QString &buildConfigurationName)
{
    foreach (XCBuildConfiguration *configuration, d->configurationList->buildConfigurations()) {
        if (configuration->name() == buildConfigurationName) {
            return configuration->buildSetting(key);
        }
    }

    return QVariant();
}

void PBXTarget::setBuildSetting(const QString &key, const QVariant &value)
{
    foreach (XCBuildConfiguration *configuration, d->configurationList->buildConfigurations())
        configuration->setBuildSetting(key, value);
}

QList<PBXBuildPhase *> PBXTarget::buildPhases() const
{
    QList<PBXBuildPhase *> phases;
    phases.append(d->frameworksBuildPhase);
    phases.append(d->sourcesBuildPhase);
    return phases;
}

PBXBuildPhase *PBXTarget::defaultFrameworksBuildPhase()
{
    return d->frameworksBuildPhase;
}

PBXBuildPhase *PBXTarget::defaultSourceCodeBuildPhase()
{
    return d->sourcesBuildPhase;
}

QString PBXTarget::comment() const
{
    return name();
}

QByteArray PBXTarget::hashData() const
{
    QByteArray hashData = PBXObject::hashData();
    hashData.append(d->name.toUtf8());
    return hashData;
}

PBXFileReference *PBXTarget::productReference() const
{
    return d->productReference;
}

void PBXTarget::setProductReference(PBXFileReference *productReference)
{
    d->productReference = productReference;
}

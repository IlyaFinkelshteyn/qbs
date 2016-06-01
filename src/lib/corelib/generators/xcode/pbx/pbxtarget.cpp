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
    QString name;
    QString productName;
    PBX::PBXProductType productType;
    XCConfigurationList *configurationList = nullptr;
    PBXFrameworksBuildPhase *frameworksBuildPhase = nullptr;
    PBXSourcesBuildPhase *sourcesBuildPhase = nullptr;
    PBXFileReference *productReference = nullptr;
    QList<PBXTargetDependency *> targetDependencies;
};

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
    self.insert(QStringLiteral("name"), name());
    self.insert(QStringLiteral("productName"), productName());
    if (!productType().isEmpty())
        self.insert(QStringLiteral("productType"), productType());
    self.insert(QStringLiteral("buildConfigurationList"),
                QVariant::fromValue(buildConfigurationList()->identifier()));

    QVariantList buildPhaseReferences;
    for (PBXBuildPhase *buildPhase : buildPhases())
        buildPhaseReferences += QVariant::fromValue(buildPhase->identifier());
    self.insert(QStringLiteral("buildPhases"), buildPhaseReferences);

    QVariantList targetDependencyReferences;
    for (PBXTargetDependency *dependency : d->targetDependencies)
        targetDependencyReferences += QVariant::fromValue(dependency->identifier());
    self.insert(QStringLiteral("dependencies"), targetDependencyReferences);

    if (d->productReference)
        self.insert(QStringLiteral("productReference"),
                    QVariant::fromValue(d->productReference->identifier()));

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
    for (XCBuildConfiguration *configuration : d->configurationList->buildConfigurations()) {
        if (configuration->name() == buildConfigurationName)
            return configuration->buildSetting(key);
    }

    return QVariant();
}

void PBXTarget::setBuildSetting(const QString &key, const QVariant &value,
                                const QString &buildConfigurationName)
{
    for (XCBuildConfiguration *configuration : d->configurationList->buildConfigurations()) {
        if (configuration->name() == buildConfigurationName) {
            configuration->setBuildSetting(key, value);
            break;
        }
    }
}

void PBXTarget::setBuildSetting(const QString &key, const QVariant &value)
{
    for (XCBuildConfiguration *configuration : d->configurationList->buildConfigurations())
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

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

#include "pbxbuildphase.h"
#include "pbxbuildrule.h"
#include "pbxbuildfile.h"
#include "pbxcontaineritemproxy.h"
#include "pbxgroup.h"
#include "pbxnativetarget.h"
#include "pbxproject.h"
#include "pbxtarget.h"
#include "pbxtargetdependency.h"
#include "xcbuildconfiguration.h"
#include "xcconfigurationlist.h"
#include <QFileInfo>
#include <QVariant>

static const QString kClassPrefixAttribute = QStringLiteral("CLASSPREFIX");
static const QString kLastUpgradeCheckAttribute = QStringLiteral("LastUpgradeCheck");
static const QString kOrganizationNameAttribute = QStringLiteral("ORGANIZATIONNAME");
static const QString kTargetAttributesAttribute = QStringLiteral("TargetAttributes");

class PBXProjectPrivate
{
public:
    QString name;
    QString projectAbsolutePath;
    QString projectDirPath;
    PBXGroup *frameworksRefGroup = nullptr;
    PBXGroup *productRefGroup = nullptr;
    QList<PBXTarget *> targets;
    XCConfigurationList *configurationList = nullptr;
    QVariantMap attributes;
};

PBXProject::PBXProject(const QString &projectAbsolutePath, QObject *parent) :
    PBXContainer(parent), d(new PBXProjectPrivate)
{
    d->projectAbsolutePath = projectAbsolutePath;
    d->configurationList = new XCConfigurationList(this);

    d->frameworksRefGroup = new PBXGroup(rootGroup());
    d->frameworksRefGroup->setName(QStringLiteral("Frameworks"));

    d->productRefGroup = new PBXGroup(rootGroup());
    d->productRefGroup->setName(QStringLiteral("Products"));
    d->productRefGroup->setSourceTree(PBX::Group);
}

PBXProject::~PBXProject()
{
    delete d;
}

QString PBXProject::isa() const
{
    return QStringLiteral("PBXProject");
}

PBXObjectMap PBXProject::toMap() const
{
    // HACK: Make sure "Frameworks", "Products" groups are always at the bottom
    rootGroup()->removeItem(d->frameworksRefGroup);
    rootGroup()->removeItem(d->productRefGroup);
    rootGroup()->addItem(d->frameworksRefGroup);
    rootGroup()->addItem(d->productRefGroup);

    PBXObjectMap self = PBXObject::toMap();
    self.insert(QStringLiteral("attributes"), d->attributes);
    self.insert(QStringLiteral("buildConfigurationList"), QVariant::fromValue(buildConfigurationList()->identifier()));
    self.insert(QStringLiteral("compatibilityVersion"), QStringLiteral("Xcode 3.2"));
    self.insert(QStringLiteral("developmentRegion"), QStringLiteral("English"));
    self.insert(QStringLiteral("hasScannedForEncodings"), QStringLiteral("1"));
    self.insert(QStringLiteral("knownRegions"), QStringList() << QStringLiteral("en"));
    self.insert(QStringLiteral("mainGroup"), QVariant::fromValue(rootGroup()->identifier()));
    self.insert(QStringLiteral("productRefGroup"), QVariant::fromValue(productRefGroup()->identifier()));
    self.insert(QStringLiteral("projectDirPath"), d->projectDirPath);
    //self.insert(QStringLiteral("projectReferences"), QVariantList()); // array of map { ProductGroup:, ProjectRef: }?
    self.insert(QStringLiteral("projectRoot"), QStringLiteral(""));

    QVariantList targetIdentifiers;
    for (PBXTarget *target : targets())
        targetIdentifiers += QVariant::fromValue(target->identifier());
    self.insert(QStringLiteral("targets"), targetIdentifiers);

    return self;
}

QString PBXProject::comment() const
{
    return QStringLiteral("Project object");
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
    for (PBXTarget *target : d->targets) {
        if (target->name() == targetName)
            return target;
    }

    return nullptr;
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

QString PBXProject::projectFilePath() const
{
    return d->projectAbsolutePath;
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
    for (XCBuildConfiguration *configuration : d->configurationList->buildConfigurations())
        configuration->setBuildSetting(key, value);
}

XCConfigurationList *PBXProject::buildConfigurationList() const
{
    return d->configurationList;
}

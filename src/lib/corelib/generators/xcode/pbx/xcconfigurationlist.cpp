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

#include "pbxproject.h"
#include "pbxtarget.h"
#include "xcbuildconfiguration.h"
#include "xcconfigurationlist.h"

class XCConfigurationListPrivate
{
public:
    QList<XCBuildConfiguration *> buildConfigurations;
    XCBuildConfiguration *defaultBuildConfiguration = nullptr;
};

XCConfigurationList::XCConfigurationList(PBXObject *parent) :
    PBXObject(parent), d(new XCConfigurationListPrivate)
{
}

XCConfigurationList::~XCConfigurationList()
{
    delete d;
}

QString XCConfigurationList::isa() const
{
    return QStringLiteral("XCConfigurationList");
}

PBXObjectMap XCConfigurationList::toMap() const
{
    PBXObjectMap self = PBXObject::toMap();

    QVariantList buildConfigurationReferences;
    for (XCBuildConfiguration *configuration : buildConfigurations())
        buildConfigurationReferences += QVariant::fromValue(configuration->identifier());
    self.insert(QStringLiteral("buildConfigurations"), buildConfigurationReferences);

    if (d->defaultBuildConfiguration)
    {
        self.insert(QStringLiteral("defaultConfigurationIsVisible"), 0);
        self.insert(QStringLiteral("defaultConfigurationName"), d->defaultBuildConfiguration->name());
    }

    return self;
}

QList<XCBuildConfiguration *> XCConfigurationList::buildConfigurations() const
{
    return d->buildConfigurations;
}

XCBuildConfiguration *XCConfigurationList::addBuildConfiguration(const QString &name)
{
    for (XCBuildConfiguration *config : d->buildConfigurations) {
        if (config->name() == name)
            return config;
    }

    XCBuildConfiguration *buildConfiguration = new XCBuildConfiguration(this);
    buildConfiguration->setName(name);
    d->buildConfigurations.append(buildConfiguration);
    return buildConfiguration;
}

void XCConfigurationList::removeBuildConfiguration(const QString &name)
{
    for (int i = 0; i < d->buildConfigurations.size(); ++i) {
        if (d->buildConfigurations[i]->name() == name) {
            XCBuildConfiguration *configuration = d->buildConfigurations.takeAt(i);
            if (d->defaultBuildConfiguration == configuration)
                d->defaultBuildConfiguration = NULL;
            delete configuration;
            break;
        }
    }
}

void XCConfigurationList::setDefaultBuildConfiguration(const QString &name)
{
    for (XCBuildConfiguration *configuration : d->buildConfigurations) {
        if (configuration->name() == name) {
            d->defaultBuildConfiguration = configuration;
            break;
        }
    }
}

QString XCConfigurationList::comment() const
{
    QString parentIsa;
    QString parentName;

    PBXTarget *target = dynamic_cast<PBXTarget *>(parent());
    PBXProject *project = dynamic_cast<PBXProject *>(parent());

    if (target) {
        parentIsa = target->isa();
        parentName = target->name();
    } else if (project) {
        parentIsa = project->isa();
        parentName = project->name();
    }

    return QString(QStringLiteral("Build configuration list for %1 \"%2\"")).arg(parentIsa).arg(parentName);
}

QByteArray XCConfigurationList::hashData() const
{
    QByteArray data = PBXObject::hashData();
    if (PBXTarget *target = dynamic_cast<PBXTarget *>(parent()))
        data.append(target->name());
    else if (PBXProject *project = dynamic_cast<PBXProject *>(parent()))
        data.append(project->name());
    return data;
}

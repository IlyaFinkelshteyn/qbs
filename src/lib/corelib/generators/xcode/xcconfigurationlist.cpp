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

#include "pbxproject.h"
#include "pbxtarget.h"
#include "xcbuildconfiguration.h"
#include "xcconfigurationlist.h"

class XCConfigurationListPrivate
{
public:
    XCConfigurationListPrivate();
    QList<XCBuildConfiguration *> buildConfigurations;
    XCBuildConfiguration *defaultBuildConfiguration;
};

XCConfigurationListPrivate::XCConfigurationListPrivate()
    : buildConfigurations(), defaultBuildConfiguration()
{
}

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
    return QLatin1String("XCConfigurationList");
}

PBXObjectMap XCConfigurationList::toMap() const
{
    PBXObjectMap self = PBXObject::toMap();

    QVariantList buildConfigurationReferences;
    foreach (XCBuildConfiguration *configuration, buildConfigurations())
        buildConfigurationReferences += QVariant::fromValue(configuration->identifier());
    self.insert(QLatin1String("buildConfigurations"), buildConfigurationReferences);

    if (d->defaultBuildConfiguration)
    {
        self.insert(QLatin1String("defaultConfigurationIsVisible"), 0);
        self.insert(QLatin1String("defaultConfigurationName"), d->defaultBuildConfiguration->name());
    }

    return self;
}

QList<XCBuildConfiguration *> XCConfigurationList::buildConfigurations() const
{
    return d->buildConfigurations;
}

XCBuildConfiguration *XCConfigurationList::addBuildConfiguration(const QString &name)
{
    foreach (XCBuildConfiguration *config, d->buildConfigurations) {
        if (config->name() == name)
            return config;
    }

    XCBuildConfiguration *buildConfiguration = new XCBuildConfiguration();
    buildConfiguration->setName(name);
    d->buildConfigurations.append(buildConfiguration);
    return buildConfiguration;
}

void XCConfigurationList::removeBuildConfiguration(const QString &name)
{
    for (int i = 0; i < d->buildConfigurations.size(); ++i)
    {
        if (d->buildConfigurations[i]->name() == name)
        {
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
    foreach (XCBuildConfiguration *configuration, d->buildConfigurations)
    {
        if (configuration->name() == name)
        {
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

    if (target)
    {
        parentIsa = target->isa();
        parentName = target->name();
    }
    else if (project)
    {
        parentIsa = project->isa();
        parentName = project->name();
    }

    return QString(QLatin1String("Build configuration list for %1 \"%2\"")).arg(parentIsa).arg(parentName);
}

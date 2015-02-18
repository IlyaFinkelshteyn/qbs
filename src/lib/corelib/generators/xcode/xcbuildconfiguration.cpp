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

#include "xcbuildconfiguration.h"
#include "xcconfigurationlist.h"

class XCBuildConfigurationPrivate
{
public:
    QString name;
    QVariantMap buildSettings;
    XCBuildConfiguration *baseConfiguration;
};

XCBuildConfiguration::XCBuildConfiguration(XCConfigurationList *parent) :
    PBXObject(parent), d(new XCBuildConfigurationPrivate)
{
    d->baseConfiguration = 0;
}

XCBuildConfiguration::~XCBuildConfiguration()
{
    delete d;
}

QString XCBuildConfiguration::isa() const
{
    return QLatin1String("XCBuildConfiguration");
}

PBXObjectMap XCBuildConfiguration::toMap() const
{
    PBXObjectMap self = PBXObject::toMap();
    if (d->baseConfiguration) {
        self.insert(QLatin1String("baseConfigurationReference"), QVariant::fromValue(d->baseConfiguration->identifier()));
    }
    self.insert(QLatin1String("buildSettings"), d->buildSettings);
    self.insert(QLatin1String("name"), d->name);
    return self;
}

QString XCBuildConfiguration::name() const
{
    return d->name;
}

void XCBuildConfiguration::setName(const QString &name)
{
    d->name = name;
}

QVariant XCBuildConfiguration::buildSetting(const QString &key)
{
    return d->buildSettings.value(key);
}

void XCBuildConfiguration::setBuildSetting(const QString &key, const QVariant &value)
{
    d->buildSettings.insert(key, value);
}

QString XCBuildConfiguration::comment() const
{
    return name();
}

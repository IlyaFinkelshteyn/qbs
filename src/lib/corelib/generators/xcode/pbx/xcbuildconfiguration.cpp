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

#include "xcbuildconfiguration.h"
#include "xcconfigurationlist.h"
#include "pbxtarget.h"
#include "pbxproject.h"

class XCBuildConfigurationPrivate
{
public:
    QString name;
    QVariantMap buildSettings;
    XCBuildConfiguration *baseConfiguration = nullptr;
};

XCBuildConfiguration::XCBuildConfiguration(XCConfigurationList *parent) :
    PBXObject(parent), d(new XCBuildConfigurationPrivate)
{
}

XCBuildConfiguration::~XCBuildConfiguration()
{
    delete d;
}

QString XCBuildConfiguration::isa() const
{
    return QStringLiteral("XCBuildConfiguration");
}

PBXObjectMap XCBuildConfiguration::toMap() const
{
    PBXObjectMap self = PBXObject::toMap();
    if (d->baseConfiguration) {
        self.insert(QStringLiteral("baseConfigurationReference"), QVariant::fromValue(d->baseConfiguration->identifier()));
    }
    self.insert(QStringLiteral("buildSettings"), d->buildSettings);
    self.insert(QStringLiteral("name"), d->name);
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

QByteArray XCBuildConfiguration::hashData() const
{
    QByteArray data = PBXObject::hashData();
    if (auto configurationList = dynamic_cast<XCConfigurationList *>(parent())) {
        if (auto target = dynamic_cast<PBXTarget *>(configurationList->parent())) {
            data.append(target->name() + QStringLiteral(":"));
        } else if (auto project = dynamic_cast<PBXProject *>(configurationList->parent())) {
            data.append(project->name() + QStringLiteral(":"));
        }
    }
    data.append(name());
    return data;
}

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
#include "pbxlegacytarget.h"
#include "pbxproject.h"

class PBXLegacyTargetPrivate
{
public:
    QString buildToolPath;
    QString buildArgumentsString;
    QString buildWorkingDirectory;
    bool passBuildSettingsInEnvironment;
};

PBXLegacyTarget::PBXLegacyTarget(PBXProject *parent) :
    PBXTarget(parent), d(new PBXLegacyTargetPrivate)
{
    d->buildToolPath = QLatin1String("/bin/bash");
    d->buildArgumentsString = QLatin1String("$(ACTION)");
    d->buildWorkingDirectory = QLatin1String("$BUILT_PRODUCTS_DIR");
    d->passBuildSettingsInEnvironment = true;
}

PBXLegacyTarget::~PBXLegacyTarget()
{
    delete d;
}

QString PBXLegacyTarget::isa() const
{
    return QLatin1String("PBXLegacyTarget");
}

PBXObjectMap PBXLegacyTarget::toMap() const
{
    PBXObjectMap self = PBXTarget::toMap();
    self.insert(QLatin1String("buildToolPath"), d->buildToolPath);
    self.insert(QLatin1String("buildArgumentsString"), d->buildArgumentsString);
    self.insert(QLatin1String("buildWorkingDirectory"), d->buildWorkingDirectory);
    self.insert(QLatin1String("passBuildSettingsInEnvironment"), d->passBuildSettingsInEnvironment ? 1 : 0);
    return self;
}

QString PBXLegacyTarget::buildToolPath() const
{
    return d->buildToolPath;
}

void PBXLegacyTarget::setBuildToolPath(const QString &buildToolPath)
{
    d->buildToolPath = buildToolPath;
}

QString PBXLegacyTarget::buildArgumentsString() const
{
    return d->buildArgumentsString;
}

void PBXLegacyTarget::setBuildArgumentsString(const QString &buildArgumentsString)
{
    d->buildArgumentsString = buildArgumentsString;
}

QString PBXLegacyTarget::buildWorkingDirectory() const
{
    return d->buildWorkingDirectory;
}

void PBXLegacyTarget::setBuildWorkingDirectory(const QString &buildWorkingDirectory)
{
    d->buildWorkingDirectory = buildWorkingDirectory;
}

bool PBXLegacyTarget::passBuildSettingsInEnvironment() const
{
    return d->passBuildSettingsInEnvironment;
}

void PBXLegacyTarget::setPassBuildSettingsInEnvironment(bool passBuildSettingsInEnvironment)
{
    d->passBuildSettingsInEnvironment = passBuildSettingsInEnvironment;
}

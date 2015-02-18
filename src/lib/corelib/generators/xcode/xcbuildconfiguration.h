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

#ifndef XCBUILDCONFIGURATION_H
#define XCBUILDCONFIGURATION_H

#include "pbxobject.h"
#include <QObject>

class PBXFileReference;
class XCConfigurationList;

class XCBuildConfigurationPrivate;

class XCBuildConfiguration : public PBXObject
{
    Q_OBJECT
public:
    explicit XCBuildConfiguration(XCConfigurationList *parent = 0);
    ~XCBuildConfiguration();

    QString isa() const;
    PBXObjectMap toMap() const;

    QString name() const;
    void setName(const QString &name);

    static bool isValidBaseConfigurationFile(PBXFileReference *reference);

    void setBaseConfigurationReference(PBXFileReference *reference);
    PBXFileReference *baseConfigurationReference();

    QVariant buildSetting(const QString &key);
    void setBuildSetting(const QString &key, const QVariant &value);

    QString comment() const;

private:
    XCBuildConfigurationPrivate *d;
};

#endif // XCBUILDCONFIGURATION_H

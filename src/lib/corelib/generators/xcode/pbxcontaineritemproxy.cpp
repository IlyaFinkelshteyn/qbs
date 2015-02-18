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

#include "pbxcontaineritemproxy.h"
#include "pbxproject.h"
#include "pbxtarget.h"
#include "pbxtargetdependency.h"

class PBXContainerItemProxyPrivate
{
public:
    PBXProject *containerPortal;
    PBXTarget *target;
};

PBXContainerItemProxy::PBXContainerItemProxy(PBXTargetDependency *parent) :
    PBXObject(parent), d(new PBXContainerItemProxyPrivate)
{
}

PBXContainerItemProxy::~PBXContainerItemProxy()
{
    delete d;
}

QString PBXContainerItemProxy::isa() const
{
    return QLatin1String("PBXContainerItemProxy");
}

PBXObjectMap PBXContainerItemProxy::toMap() const
{
    PBXObjectMap self = PBXObject::toMap();
    if (d->containerPortal)
        self.insert(QLatin1String("containerPortal"), QVariant::fromValue(d->containerPortal->identifier()));

    self.insert(QLatin1String("proxyType"), 1); // TODO: Always the same?

    if (d->target) {
        // We deliberately don't store the actual PBXObjectIdentifier,
        // because Xcode itself doesn't write the ID with a comment annotation
        self.insert(QLatin1String("remoteGlobalIDString"), d->target->identifier().identifier());
        self.insert(QLatin1String("remoteInfo"), d->target->name());
    }

    return self;
}

QString PBXContainerItemProxy::comment() const
{
    return PBXObject::comment();
}

QByteArray PBXContainerItemProxy::hashData() const
{
    QByteArray data = PBXObject::hashData();
    if (d->containerPortal) {
        data.append(d->containerPortal->hashData());
    }

    if (d->target) {
        data.append(d->target->hashData());
    }

    return data;
}

PBXProject *PBXContainerItemProxy::containerPortal() const
{
    return d->containerPortal;
}

void PBXContainerItemProxy::setContainerPortal(PBXProject *containerPortal)
{
    d->containerPortal = containerPortal;
}

PBXTarget *PBXContainerItemProxy::target() const
{
    return d->target;
}

void PBXContainerItemProxy::setTarget(PBXTarget *target)
{
    d->target = target;
}

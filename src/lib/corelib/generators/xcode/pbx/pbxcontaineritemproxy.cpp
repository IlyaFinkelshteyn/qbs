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

#include "pbxcontaineritemproxy.h"
#include "pbxproject.h"
#include "pbxtarget.h"
#include "pbxtargetdependency.h"

class PBXContainerItemProxyPrivate
{
public:
    PBXProject *containerPortal = nullptr;
    PBXTarget *target = nullptr;
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
    return QStringLiteral("PBXContainerItemProxy");
}

PBXObjectMap PBXContainerItemProxy::toMap() const
{
    PBXObjectMap self = PBXObject::toMap();
    if (d->containerPortal)
        self.insert(QStringLiteral("containerPortal"), QVariant::fromValue(d->containerPortal->identifier()));

    self.insert(QStringLiteral("proxyType"), 1); // TODO: Always the same?

    if (d->target) {
        // We deliberately don't store the actual PBXObjectIdentifier,
        // because Xcode itself doesn't write the ID with a comment annotation
        self.insert(QStringLiteral("remoteGlobalIDString"), d->target->identifier().identifier());
        self.insert(QStringLiteral("remoteInfo"), d->target->name());
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

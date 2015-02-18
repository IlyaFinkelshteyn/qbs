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

class PBXTargetDependencyPrivate
{
public:
    PBXTargetDependencyPrivate();
    PBXContainerItemProxy *targetProxy;
};

PBXTargetDependencyPrivate::PBXTargetDependencyPrivate()
    : targetProxy()
{
}

PBXTargetDependency::PBXTargetDependency(PBXProject *parent)
    : PBXObject(parent), d(new PBXTargetDependencyPrivate)
{
    d->targetProxy = new PBXContainerItemProxy(this);
    d->targetProxy->setContainerPortal(parent);
}

PBXTargetDependency::~PBXTargetDependency()
{
    delete d;
}

QString PBXTargetDependency::isa() const
{
    return QLatin1String("PBXTargetDependency");
}

PBXObjectMap PBXTargetDependency::toMap() const
{
    PBXObjectMap self = PBXObject::toMap();
    if (d->targetProxy) {
        self.insert(QLatin1String("targetProxy"), QVariant::fromValue(d->targetProxy->identifier()));
        if (d->targetProxy->target())
            self.insert(QLatin1String("target"), QVariant::fromValue(d->targetProxy->target()->identifier()));
    }
    return self;
}

QString PBXTargetDependency::comment() const
{
    return PBXObject::comment();
}

PBXTarget *PBXTargetDependency::target() const
{
    return d->targetProxy->target();
}

void PBXTargetDependency::setTarget(PBXTarget *target)
{
    d->targetProxy->setTarget(target);
}

PBXContainerItemProxy *PBXTargetDependency::targetProxy() const
{
    return d->targetProxy;
}

void PBXTargetDependency::setTargetProxy(PBXContainerItemProxy *targetProxy)
{
    d->targetProxy = targetProxy;
}

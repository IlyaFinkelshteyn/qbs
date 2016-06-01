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
#include "pbxfilereference.h"
#include "pbxnativetarget.h"
#include "pbxproject.h"
#include "xcconfigurationlist.h"

class PBXNativeTargetPrivate
{
public:
    QList<PBXBuildRule *> buildRules;
};

PBXNativeTarget::PBXNativeTarget(PBXProject *parent) :
    PBXTarget(parent), d(new PBXNativeTargetPrivate)
{
}

PBXNativeTarget::~PBXNativeTarget()
{
    delete d;
}

QString PBXNativeTarget::isa() const
{
    return QStringLiteral("PBXNativeTarget");
}

PBXObjectMap PBXNativeTarget::toMap() const
{
    // This cannot be empty (and will crash Xcode if it is),
    // and is only used for native targets (not legacy targets),
    // though for the latter it will influence icons, which is nice.
    if (productType().isEmpty())
        throw std::runtime_error("PBXTarget::productType cannot be empty");

    PBXObjectMap self = PBXTarget::toMap();
    self.insert(QStringLiteral("productInstallPath"), QStringLiteral("$(HOME)/Applications"));

    QVariantList buildRuleReferences;
    for (PBXBuildRule *buildRule : buildRules())
        buildRuleReferences += QVariant::fromValue(buildRule->identifier());
    self.insert(QStringLiteral("buildRules"), buildRuleReferences);

    return self;
}

QList<PBXBuildRule *> PBXNativeTarget::buildRules() const
{
    return d->buildRules;
}

void PBXNativeTarget::addBuildRule(PBXBuildRule *rule)
{
    d->buildRules.append(rule);
}

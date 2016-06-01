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

#include "pbxbuildfile.h"
#include "pbxbuildphase.h"
#include "pbxfilereference.h"
#include "pbxtarget.h"

class PBXBuildPhasePrivate
{
public:
    QList<PBXBuildFile *> buildFiles;
};

PBXBuildPhase::PBXBuildPhase(PBXTarget *parent) :
    PBXObject(parent), d(new PBXBuildPhasePrivate)
{
}

PBXBuildPhase::~PBXBuildPhase()
{
    delete d;
}

PBXObjectMap PBXBuildPhase::toMap() const
{
    PBXObjectMap self = PBXObject::toMap();
    self.insert(QStringLiteral("buildActionMask"), std::numeric_limits<int>::max());

    QVariantList fileReferenceList;
    for (PBXBuildFile *buildFile : buildFiles())
        fileReferenceList += QVariant::fromValue(buildFile->identifier());
    self.insert(QStringLiteral("files"), fileReferenceList);

    self.insert(QStringLiteral("runOnlyForDeploymentPostprocessing"), 0);

    return self;
}

QByteArray PBXBuildPhase::hashData() const
{
    QByteArray data;
    PBXTarget *target = dynamic_cast<PBXTarget *>(parent());
    if (target)
        data.append(target->hashData());
    return data; // TODO: Build phases for which a target may have multiple of, will not work
}

QList<PBXBuildFile *> PBXBuildPhase::buildFiles() const
{
    return d->buildFiles;
}

PBXBuildFile *PBXBuildPhase::addReference(PBXFileReference *reference)
{
    for (PBXBuildFile *buildFile : d->buildFiles) {
        if (buildFile->fileReference() == reference)
            return buildFile;
    }

    PBXBuildFile *buildFile = new PBXBuildFile(this);
    buildFile->setFileReference(reference);
    d->buildFiles.append(buildFile);
    return buildFile;
}

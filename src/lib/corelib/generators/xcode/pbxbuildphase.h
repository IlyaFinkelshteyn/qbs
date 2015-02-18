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

#ifndef PBXBUILDPHASE_H
#define PBXBUILDPHASE_H

#include "pbxobject.h"
#include <QObject>

class PBXBuildFile;
class PBXFileReference;
class PBXTarget;

class PBXBuildPhasePrivate;

class PBXBuildPhase : public PBXObject
{
    Q_OBJECT
public:
    explicit PBXBuildPhase(PBXTarget *parent = 0);
    virtual ~PBXBuildPhase();

    virtual QString name() const = 0;

    PBXObjectMap toMap() const;

    QByteArray hashData() const;

    QList<PBXBuildFile *> buildFiles() const;

    PBXBuildFile *addReference(PBXFileReference *reference);
    bool containsFileReferenceIdenticalTo(PBXFileReference *reference);

private:
    PBXBuildPhasePrivate *d;
};

#endif // PBXBUILDPHASE_H

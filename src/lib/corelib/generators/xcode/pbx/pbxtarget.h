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

#ifndef PBXTARGET_H
#define PBXTARGET_H

#include "pbxobject.h"
#include "pbxproducttype.h"
#include <QObject>

class PBXBuildPhase;
class PBXFileReference;
class PBXProject;
class PBXTargetDependency;
class XCConfigurationList;

class PBXTargetPrivate;

class PBXTarget : public PBXObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PBXTarget)
public:
    explicit PBXTarget(PBXProject *parent = 0);
    ~PBXTarget();

    PBXObjectMap toMap() const;

    QString name() const;
    void setName(const QString &name);

    QString productName() const;
    void setProductName(const QString &productName);

    PBX::PBXProductType productType() const;
    void setProductType(PBX::PBXProductType productType);

    /*!
     * File reference to the target's output file.
     */
    PBXFileReference *productReference() const;
    void setProductReference(PBXFileReference *productReference);

    /*!
     * List of the target's build configurations. This is never NULL.
     */
    XCConfigurationList *buildConfigurationList() const;

    QList<PBXTargetDependency *> &targetDependencies() const;

    QString expandedValue(const QString &string, void *buildParameters) const;

    QVariant buildSetting(const QString &key, const QString &buildConfigurationName);
    void setBuildSetting(const QString &key, const QVariant &value,
                         const QString &buildConfigurationName);
    void setBuildSetting(const QString &key, const QVariant &value);

    QList<PBXBuildPhase *> buildPhases() const;
    PBXBuildPhase *buildPhaseOfClass(void *clazz) const;
    void addBuildPhase(PBXBuildPhase *buildPhase);
    PBXBuildPhase *defaultFrameworksBuildPhase();
    PBXBuildPhase *defaultLinkBuildPhase();
    PBXBuildPhase *defaultSourceCodeBuildPhase();
    PBXBuildPhase *defaultResourceBuildPhase();
    PBXBuildPhase *defaultHeaderBuildPhase();

    QString comment() const;

    QByteArray hashData() const;

private:
    PBXTargetPrivate *d;
};

#endif // PBXTARGET_H

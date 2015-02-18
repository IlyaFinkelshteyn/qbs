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

#ifndef QBS_XCODEGENERATOR_H
#define QBS_XCODEGENERATOR_H

#include <api/projectdata.h>
#include <generators/generator.h>
#include "pbxproducttype.h"

#include <QDir>

class PBXFileReference;
class PBXGroup;
class PBXProject;
class PBXTarget;

namespace qbs {

class ProductData;
struct XcodeBuildSettingMapping;

class XcodeGenerator : public qbs::ProjectGenerator
{
public:
    virtual void generate(const InstallOptions &installOptions);
    virtual void addProjectRecursive(const Project &project, const ProjectData &projectData, const InstallOptions &installOptions, PBXProject &xcodeProject, PBXGroup *xcodeGroup);
    virtual void addProduct(const Project &project, const ProductData &product, const InstallOptions &installOptions, PBXProject &xcodeProject, PBXGroup *xcodeGroup);
    virtual QList<PBXFileReference *> addGroupFiles(PBXProject *xcodeProject, PBXGroup *xcodeGroup, const qbs::GroupData &groupData);

    virtual PBXTarget *xcodeTargetForProduct(PBXProject *xcodeProject, const ProductData &product);

    QString xcodeConfigurationName(const Project &project);
    QString xcodeConfigurationName(const ProductData &product);

    static QVariant xcodeInsertPlaceholdersInValue(const qbs::Project &project, const QVariant &variant);

    static PBX::PBXProductType xcodeProductType(const ProductData &product);

protected:
    XcodeGenerator();

    // TODO: Move to base class
    virtual void setupGenerator();
    bool m_multipleProfiles;
    QString m_projectName;
    QFileInfo m_qbsProjectFile;
    QDir m_baseBuildDirectory;
    QMap<QString, PBXGroup*> m_productGroups;
};

}

#endif // QBS_XCODEGENERATOR_H

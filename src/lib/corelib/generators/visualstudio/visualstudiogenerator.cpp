/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing
**
** This file is part of Qbs.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms and
** conditions see http://www.qt.io/terms-conditions. For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, The Qt Company gives you certain additional
** rights.  These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "msbuildfiltersproject.h"
#include "msbuildqbsgenerateproject.h"
#include "msbuildsharedsolutionpropertiesproject.h"
#include "msbuildsolutionpropertiesproject.h"
#include "msbuildqbsproductproject.h"
#include "msbuildutils.h"
#include "visualstudiogenerator.h"
#include "visualstudioguidpool.h"

#include "msbuild/msbuildpropertygroup.h"
#include "msbuild/msbuildproject.h"

#include "solution/visualstudiosolution.h"
#include "solution/visualstudiosolutionfileproject.h"
#include "solution/visualstudiosolutionglobalsection.h"
#include "solution/visualstudiosolutionfolderproject.h"

#include "io/msbuildprojectwriter.h"
#include "io/visualstudiosolutionwriter.h"

#include <generators/generatableprojectiterator.h>
#include <logging/translator.h>
#include <tools/filesaver.h>
#include <tools/qbsassert.h>
#include <tools/shellutils.h>
#include <tools/visualstudioversioninfo.h>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace qbs {

using namespace Internal;

class VisualStudioGeneratorPrivate
{
    friend class SolutionDependenciesVisitor;
public:
    VisualStudioGeneratorPrivate(const Internal::VisualStudioVersionInfo &versionInfo)
        : versionInfo(versionInfo) {}

    Internal::VisualStudioVersionInfo versionInfo;

    QSharedPointer<VisualStudioGuidPool> guidPool;
    QSharedPointer<VisualStudioSolution> solution;
    QString solutionFilePath;
    QMap<QString, QSharedPointer<MSBuildProject>> msbuildProjects;
    QMap<QString, VisualStudioSolutionFileProject *> solutionProjects;
    QMap<QString, VisualStudioSolutionFolderProject *> solutionFolders;
    QList<QPair<QString, bool>> propertySheetNames;

    void reset();
};

void VisualStudioGeneratorPrivate::reset()
{
    guidPool.reset();
    solution.reset();
    solutionFilePath.clear();
    msbuildProjects.clear();
    solutionProjects.clear();
    solutionFolders.clear();
    propertySheetNames.clear();
}

class SolutionDependenciesVisitor : public IGeneratableProjectVisitor
{
public:
    SolutionDependenciesVisitor(VisualStudioGenerator *generator)
        : generator(generator) {
    }

    void visitProject(const GeneratableProject &project) override {
        Q_UNUSED(project);
        nestedProjects = new VisualStudioSolutionGlobalSection(
                    QStringLiteral("NestedProjects"), generator->d->solution.data());
        generator->d->solution->appendGlobalSection(nestedProjects);
    }

    void visitProjectData(const GeneratableProject &project,
                          const GeneratableProjectData &parentProjectData,
                          const GeneratableProjectData &projectData) override {
        Q_UNUSED(project);
        // The root project will have a null GeneratableProjectData
        // as its parent object (so skip giving it a parent folder)
        if (!parentProjectData.name().isEmpty()) {
            nestedProjects->appendProperty(
                        generator->d->solutionFolders.value(projectData.name())->guid()
                            .toString(),
                        generator->d->solutionFolders.value(parentProjectData.name())->guid()
                            .toString());
        }
    }

    void visitProduct(const GeneratableProject &project,
                      const GeneratableProjectData &projectData,
                      const GeneratableProductData &productData) override {
        Q_UNUSED(project);
        Q_UNUSED(projectData);
        for (const auto &dep : productData.dependencies()) {
            generator->d->solution->addDependency(
                        generator->d->solutionProjects.value(productData.name()),
                        generator->d->solutionProjects.value(dep));
        }

        nestedProjects->appendProperty(
                    generator->d->solutionProjects.value(productData.name())->guid().toString(),
                    generator->d->solutionFolders.value(projectData.name())->guid().toString());
    }

private:
    VisualStudioGenerator *generator;
    VisualStudioSolutionGlobalSection *nestedProjects;
};

VisualStudioGenerator::VisualStudioGenerator(const VisualStudioVersionInfo &versionInfo)
    : d(new VisualStudioGeneratorPrivate(versionInfo))
{
    if (d->versionInfo.usesVcBuild())
        throw ErrorInfo(Tr::tr("VCBuild (Visual Studio 2008 and below) is not supported"));
    else if (!d->versionInfo.usesMsBuild())
        throw ErrorInfo(Tr::tr("Unknown/unsupported build engine"));
    Q_ASSERT(d->versionInfo.usesSolutions());
}

VisualStudioGenerator::~VisualStudioGenerator()
{
}

QString VisualStudioGenerator::generatorName() const
{
    return QStringLiteral("visualstudio%1").arg(d->versionInfo.marketingVersion());
}

void VisualStudioGenerator::addPropertySheets(const GeneratableProject &project)
{
    {
        const auto fileName = QStringLiteral("qbs.props");
        d->propertySheetNames.append({ fileName, true });
        d->msbuildProjects.insert(project.baseBuildDirectory().absoluteFilePath(fileName),
                                QSharedPointer<MSBuildSolutionPropertiesProject>::create(
                                    d->versionInfo, project, qbsExecutableFilePath()));
    }

    {
        const auto fileName = QStringLiteral("qbs-shared.props");
        d->propertySheetNames.append({ fileName, false });
        d->msbuildProjects.insert(project.baseBuildDirectory().absoluteFilePath(fileName),
                                QSharedPointer<MSBuildSharedSolutionPropertiesProject>::create(
                                    d->versionInfo, project, qbsExecutableFilePath()));
    }
}

void VisualStudioGenerator::addPropertySheets(
        const QSharedPointer<MSBuildTargetProject> &targetProject)
{
    for (const auto &pair : d->propertySheetNames) {
        targetProject->appendPropertySheet(
                    QStringLiteral("$(SolutionDir)\\") + pair.first, pair.second);
    }
}

static QString targetFilePath(const QString &baseName, const QString &baseBuildDirectory)
{
    return QDir(baseBuildDirectory).absoluteFilePath(baseName + QStringLiteral(".vcxproj"));
}

static QString targetFilePath(const GeneratableProductData &product,
                              const QString &baseBuildDirectory)
{
    return targetFilePath(product.name(), baseBuildDirectory);
}

static void addDefaultGlobalSections(const GeneratableProject &topLevelProject,
                                     VisualStudioSolution *solution)
{
    auto configurationPlatformsSection = new VisualStudioSolutionGlobalSection(
                QStringLiteral("SolutionConfigurationPlatforms"), solution);
    solution->appendGlobalSection(configurationPlatformsSection);
    for (const auto &qbsProject : topLevelProject.projects)
        configurationPlatformsSection->appendProperty(MSBuildUtils::fullName(qbsProject),
                                                      MSBuildUtils::fullName(qbsProject));

    auto projectConfigurationPlatformsSection = new VisualStudioSolutionGlobalSection(
                QStringLiteral("ProjectConfigurationPlatforms"), solution);
    solution->appendGlobalSection(projectConfigurationPlatformsSection);
    projectConfigurationPlatformsSection->setPost(true);
    for (const auto project : solution->projects()) {
        for (const auto &qbsProject : topLevelProject.projects) {
            projectConfigurationPlatformsSection->appendProperty(
                QStringLiteral("%1.%2.ActiveCfg")
                        .arg(project->guid().toString())
                        .arg(MSBuildUtils::fullDisplayName(qbsProject)),
                MSBuildUtils::fullName(qbsProject));
            projectConfigurationPlatformsSection->appendProperty(
                QStringLiteral("%1.%2.Build.0")
                        .arg(project->guid().toString())
                        .arg(MSBuildUtils::fullDisplayName(qbsProject)),
                MSBuildUtils::fullName(qbsProject));
        }
    }

    auto solutionPropsSection = new VisualStudioSolutionGlobalSection(
                QStringLiteral("SolutionProperties"), solution);
    solution->appendGlobalSection(solutionPropsSection);
    solutionPropsSection->appendProperty(QStringLiteral("HideSolutionNode"),
                                         QStringLiteral("FALSE"));
}

static void writeProjectFiles(const QMap<QString, QSharedPointer<MSBuildProject>> &projects)
{
    // Write out all the MSBuild project files to disk
    QMapIterator<QString, QSharedPointer<MSBuildProject>> it(projects);
    while (it.hasNext()) {
        it.next();
        const auto projectFilePath = it.key();
        Internal::FileSaver file(projectFilePath);
        if (!file.open())
            throw ErrorInfo(Tr::tr("Cannot open %s for writing").arg(projectFilePath));

        QSharedPointer<MSBuildProject> project = it.value();
        MSBuildProjectWriter writer(file.device());
        if (!(writer.write(project.data()) && file.commit()))
            throw ErrorInfo(Tr::tr("Failed to generate %1").arg(projectFilePath));
    }
}

static void writeSolution(const QSharedPointer<VisualStudioSolution> &solution,
                          const QString &solutionFilePath)
{
    Internal::FileSaver file(solutionFilePath);
    if (!file.open())
        throw ErrorInfo(Tr::tr("Cannot open %s for writing").arg(solutionFilePath));

    VisualStudioSolutionWriter writer(file.device());
    writer.setProjectBaseDirectory(QFileInfo(solutionFilePath).path());
    if (!(writer.write(solution.data()) && file.commit()))
        throw ErrorInfo(Tr::tr("Failed to generate %1").arg(solutionFilePath));

    qDebug() << "Generated" << qPrintable(QFileInfo(solutionFilePath).fileName());
}

void VisualStudioGenerator::generate()
{
    GeneratableProjectIterator it(project());
    it.accept(this);

    addDefaultGlobalSections(project(), d->solution.data());

    // Second pass: connection solution project interdependencies and project nesting hierarchy
    SolutionDependenciesVisitor solutionDependenciesVisitor(this);
    it.accept(&solutionDependenciesVisitor);

    writeProjectFiles(d->msbuildProjects);
    writeSolution(d->solution, d->solutionFilePath);

    d->reset();
}

QVector<QSharedPointer<ProjectGenerator> > VisualStudioGenerator::createGeneratorList()
{
    QVector<QSharedPointer<ProjectGenerator> > result;
    for (const auto &info : VisualStudioVersionInfo::knownVersions()) {
        if (info.usesMsBuild())
            result << QSharedPointer<ProjectGenerator>(new VisualStudioGenerator(info));
    }
    return result;
}

void VisualStudioGenerator::visitProject(const GeneratableProject &project)
{
    addPropertySheets(project);

    const auto buildDir = project.baseBuildDirectory();

    d->guidPool = QSharedPointer<VisualStudioGuidPool>::create(
                buildDir.absoluteFilePath(project.name() + QStringLiteral(".guid.txt")));

    d->solutionFilePath = buildDir.absoluteFilePath(project.name() + QStringLiteral(".sln"));
    d->solution = QSharedPointer<VisualStudioSolution>::create(d->versionInfo);

    // Create a helper project to re-run qbs generate
    const auto qbsGenerate = QStringLiteral("qbs-generate");
    const auto projectFilePath = targetFilePath(qbsGenerate, buildDir.absolutePath());
    const auto relativeProjectFilePath = QFileInfo(d->solutionFilePath).dir()
            .relativeFilePath(projectFilePath);
    auto targetProject = QSharedPointer<MSBuildQbsGenerateProject>::create(project, d->versionInfo);
    targetProject->setGuid(d->guidPool->drawProductGuid(relativeProjectFilePath));
    d->msbuildProjects.insert(projectFilePath, targetProject);

    addPropertySheets(targetProject);

    auto solutionProject = new VisualStudioSolutionFileProject(
                targetFilePath(qbsGenerate, project.baseBuildDirectory().absolutePath()),
                d->solution.data());
    solutionProject->setGuid(targetProject->guid());
    d->solution->appendProject(solutionProject);
    d->solutionProjects.insert(qbsGenerate, solutionProject);
}

void VisualStudioGenerator::visitProjectData(const GeneratableProject &project,
                                             const GeneratableProjectData &projectData)
{
    Q_UNUSED(project);
    auto solutionFolder = new VisualStudioSolutionFolderProject(d->solution.data());
    solutionFolder->setName(projectData.name());
    d->solution->appendProject(solutionFolder);
    d->solutionFolders.insert(projectData.name(), solutionFolder);
}

void VisualStudioGenerator::visitProduct(const GeneratableProject &project,
                                         const GeneratableProjectData &projectData,
                                         const GeneratableProductData &productData)
{
    Q_UNUSED(projectData);
    const auto projectFilePath = targetFilePath(productData,
                                                project.baseBuildDirectory().absolutePath());
    const auto relativeProjectFilePath = QFileInfo(d->solutionFilePath)
            .dir().relativeFilePath(projectFilePath);
    auto targetProject = QSharedPointer<MSBuildQbsProductProject>::create(project, productData,
                                                                          d->versionInfo);
    targetProject->setGuid(d->guidPool->drawProductGuid(relativeProjectFilePath));

    addPropertySheets(targetProject);

    d->msbuildProjects.insert(projectFilePath, targetProject);
    d->msbuildProjects.insert(projectFilePath + QStringLiteral(".filters"),
                          QSharedPointer<MSBuildFiltersProject>::create(productData));

    auto solutionProject = new VisualStudioSolutionFileProject(
                targetFilePath(productData, project.baseBuildDirectory().absolutePath()),
                d->solution.data());
    solutionProject->setGuid(targetProject->guid());
    d->solution->appendProject(solutionProject);
    d->solutionProjects.insert(productData.name(), solutionProject);
}

} // namespace qbs

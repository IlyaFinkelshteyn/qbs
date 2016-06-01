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

#ifndef GENERATORDATA_H
#define GENERATORDATA_H

#include <QDir>
#include <QMap>
#include <api/project.h>
#include <api/projectdata.h>
#include <tools/error.h>
#include <functional>

namespace qbs {

typedef QMap<QString, Project> GeneratableProjectMap;
typedef QMap<QString, ProjectData> GeneratableProjectDataMap;
typedef QMap<QString, ProductData> GeneratableProductDataMap;

template <typename U> struct IMultiplexableContainer {
    QMap<QString, U> data;

    template <typename T> T uniqueValue(const std::function<T(const U &data)> &func,
                                        const QString &errorMessage) const
    {
        QSet<T> values;
        for (const auto &productData : data) {
            values.insert(func(productData));
            if (values.size() > 1)
                throw ErrorInfo(errorMessage);
        }
        return values.toList().first();
    }

    void forEach(const std::function<void(const QString &configurationName,
                                          const U &data)> &func) const
    {
        QMapIterator<QString, U> it(data);
        while (it.hasNext()) {
            it.next();
            func(it.key(), it.value());
        }
    }

    bool isValid() const
    {
        return !data.isEmpty();
    }

protected:
    IMultiplexableContainer() { }
};

struct GeneratableProductData : public IMultiplexableContainer<ProductData> {
    QString name() const;
    QStringList type() const;
    QStringList dependencies() const;
    CodeLocation location() const;

    const ProductData operator[](const QString &configurationName)
    {
        return data[configurationName];
    }
};

struct GeneratableProjectData : public IMultiplexableContainer<ProjectData> {
    QList<GeneratableProjectData> subProjects;
    QList<GeneratableProductData> products;
    QString name() const;

    const ProjectData operator[](const QString &configurationName)
    {
        return data[configurationName];
    }
};

struct GeneratableProject : public GeneratableProjectData {
    GeneratableProjectMap projects;
    QMap<QString, QVariantMap> buildConfigurations;
    QMap<QString, QStringList> commandLines;
    QString installRoot;
    QDir baseBuildDirectory() const;
    QFileInfo filePath() const;
    bool hasMultipleConfigurations() const;
    QStringList commandLine() const;

    const Project operator[](const QString &configurationName) const
    {
        return projects[configurationName];
    }

    const ProjectData projectData(const QString &configurationName) const
    {
        return data[configurationName];
    }

    void forEach(const std::function<void(const QString &configurationName,
                                          const Project &data)> &func) const
    {
        QMapIterator<QString, Project> it(projects);
        while (it.hasNext()) {
            it.next();
            func(it.key(), it.value());
        }
    }
};

} // namespace qbs

#endif // GENERATORDATA_H

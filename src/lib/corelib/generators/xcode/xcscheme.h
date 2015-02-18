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

#ifndef XCSCHEME_H
#define XCSCHEME_H

#include <QObject>

class PBXProject;
class PBXTarget;

class XCSchemePrivate;

class XCScheme : public QObject
{
    Q_OBJECT
public:
    explicit XCScheme(QObject *parent = 0);
    virtual ~XCScheme();

    PBXProject *project() const;
    void setProject(PBXProject *project);

    PBXTarget *target() const;
    void setTarget(PBXTarget *target);

    bool parallelizeBuildables() const;
    void setParallelizeBuildables(bool parallelize);

    bool buildImplicitDependencies() const;
    void setBuildImplicitDependencies(bool build);

    QString customExecutableFilePath() const;
    void setCustomExecutableFilePath(const QString &customExecutableFilePath);

    bool serialize(const QString &filePath);

private:
    XCSchemePrivate *d;
};

#endif // XCSCHEME_H

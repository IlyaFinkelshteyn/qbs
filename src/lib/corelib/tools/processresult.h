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
#ifndef QBS_PROCESSRESULT_H
#define QBS_PROCESSRESULT_H

#include "qbs_export.h"

#include <QExplicitlySharedDataPointer>
#include <QMetaType>
#include <QProcess>
#include <QString>
#include <QStringList>

namespace qbs {
namespace Internal {
class ProcessCommandExecutor;
class ProcessResultPrivate;
}

class QBS_EXPORT ProcessResult
{
    friend class qbs::Internal::ProcessCommandExecutor;
public:
    ProcessResult();
    ProcessResult(const ProcessResult &other);
    ProcessResult &operator=(const ProcessResult &other);
    ~ProcessResult();

    bool success() const;
    QString executableFilePath() const;
    QStringList arguments() const;
    QString workingDirectory() const;
    QProcess::ProcessError error() const;
    int exitCode() const;
    QStringList stdOut() const;
    QStringList stdErr() const;

private:
    QExplicitlySharedDataPointer<Internal::ProcessResultPrivate> d;
};

} // namespace qbs

Q_DECLARE_METATYPE(qbs::ProcessResult)

#endif // QBS_PROCESSRESULT_H

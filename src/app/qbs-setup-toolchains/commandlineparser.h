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
#ifndef QBS_SETUPTOOLCHAINS_COMMANDLINEPARSER_H
#define QBS_SETUPTOOLCHAINS_COMMANDLINEPARSER_H

#include <QStringList>

class CommandLineParser
{
public:
    void parse(const QStringList &commandLine);

    bool helpRequested() const { return m_helpRequested; }
    bool autoDetectionMode() const { return m_autoDetectionMode; }

    QString compilerPath() const { return m_compilerPath; }
    QString toolchainType() const { return m_toolchainType; }
    QString profileName() const { return m_profileName; }
    QString settingsDir() const { return m_settingsDir; }

    QString usageString() const;

private:
    void throwError(const QString &message);
    void assignOptionArgument(const QString &option, QString &argument);
    void complainAboutExtraArguments();

    bool m_helpRequested;
    bool m_autoDetectionMode;
    QString m_compilerPath;
    QString m_toolchainType;
    QString m_profileName;
    QString m_settingsDir;
    QStringList m_commandLine;
    QString m_command;
};

#endif // Include guard

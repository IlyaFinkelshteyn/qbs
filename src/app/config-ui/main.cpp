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
#include "mainwindow.h"

#include "commandlineparser.h"

#include <logging/translator.h>
#include <tools/error.h>

#include <QApplication>
#include <cstdlib>
#include <iostream>

using qbs::Internal::Tr;

extern "C" void qt_macos_forceTransformProcessToForegroundApplicationAndActivate();

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    CommandLineParser clParser;
    try {
        clParser.parse(app.arguments());
        if (clParser.helpRequested()) {
            std::cout << qPrintable(clParser.usageString());
            return EXIT_SUCCESS;
        }
    } catch (const qbs::ErrorInfo &error) {
        std::cerr << qPrintable(error.toString());
        return EXIT_FAILURE;
    }

    // Effectively delay the foreground process transformation from QApplication construction to
    // just before UI is shown - this prevents the application icon from popping up in the Dock
    // when running `qbs help`, and QCoreApplication::arguments() requires the application object
    // to be constructed, so it is not easily worked around
#if defined(Q_OS_MACOS) || defined(Q_OS_OSX)
    qt_macos_forceTransformProcessToForegroundApplicationAndActivate();
#endif
    MainWindow mw(clParser.settingsDir());
    mw.show();
    return app.exec();
}

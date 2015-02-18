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

#include "pbxproducttype.h"

QString PBX::productTypeString(PBX::PBXProductType productType)
{
    switch (productType)
    {
    case Application:
        return QLatin1String("com.apple.product-type.application");
    case Tool:
        return QLatin1String("com.apple.product-type.tool");
    case LibraryStatic:
        return QLatin1String("com.apple.product-type.library.static");
    case LibraryDynamic:
        return QLatin1String("com.apple.product-type.library.dynamic");
    case Framework:
        return QLatin1String("com.apple.product-type.framework");
    case StaticFramework:
        return QLatin1String("com.apple.product-type.framework.static");
    case Bundle:
        return QLatin1String("com.apple.product-type.bundle");
    case KernelExtension:
        return QLatin1String("com.apple.product-type.kernel-extension");
    case KernelExtensionIOKit:
        return QLatin1String("com.apple.product-type.kernel-extension.iokit");
    }

    QLatin1String("com.apple.product-type.application.java");
    QLatin1String("com.apple.product-type.application.shallow");
    QLatin1String("com.apple.product-type.app-extension");
    QLatin1String("com.apple.product-type.bundle.shallow");
    QLatin1String("com.apple.product-type.bundle.unit-test");
    QLatin1String("com.apple.product-type.bundle.ocunit-test");
    QLatin1String("com.apple.product-type.framework.shallow");
    QLatin1String("com.apple.product-type.in-app-purchase-content");
    QLatin1String("com.apple.product-type.kernel-extension.shallow");
    QLatin1String("com.apple.product-type.kernel-extension.iokit.shallow");
    QLatin1String("com.apple.product-type.library.java.archive");
    QLatin1String("com.apple.product-type.objfile");
    QLatin1String("com.apple.product-type.pluginkit-plugin");
    QLatin1String("com.apple.product-type.spotlight-importer");
    QLatin1String("com.apple.product-type.tool.java");
    QLatin1String("com.apple.product-type.xpc-service");

    return QString();
}

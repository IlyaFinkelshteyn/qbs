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

#include "pbxproducttype.h"

namespace PBX {
    const PBXProductType PBXApplicationProductType = QStringLiteral("com.apple.product-type.application");
    const PBXProductType PBXBundleProductType = QStringLiteral("com.apple.product-type.bundle");
    const PBXProductType PBXDynamicLibraryProductType = QStringLiteral("com.apple.product-type.library.dynamic");
    const PBXProductType PBXFrameworkProductType = QStringLiteral("com.apple.product-type.framework");
    const PBXProductType PBXShallowBundleProductType = QStringLiteral("com.apple.product-type.bundle.shallow");
    const PBXProductType PBXShallowFrameworkProductType = QStringLiteral("com.apple.product-type.framework.shallow");
    const PBXProductType PBXStaticLibraryProductType = QStringLiteral("com.apple.product-type.library.static");
    const PBXProductType PBXToolProductType = QStringLiteral("com.apple.product-type.tool");
    const PBXProductType XCKernelExtensionProductType = QStringLiteral("com.apple.product-type.kernel-extension");
    const PBXProductType XCShallowKernelExtensionProductType = QStringLiteral("com.apple.product-type.kernel-extension.shallow");
    const PBXProductType XCStaticFrameworkProductType = QStringLiteral("com.apple.product-type.framework.static");

    const PBXProductType XCApplicationExtensionProductType = QStringLiteral("com.apple.product-type.app-extension");
    const PBXProductType XCInAppPurchaseContentProductType = QStringLiteral("com.apple.product-type.in-app-purchase-content");
    const PBXProductType XCXPCServiceProductType = QStringLiteral("com.apple.product-type.xpc-service");

    //QStringLiteral("com.apple.product-type.application.java");
    //QStringLiteral("com.apple.product-type.application.shallow");
    //QStringLiteral("com.apple.product-type.bundle.unit-test");
    //QStringLiteral("com.apple.product-type.bundle.ocunit-test");
    //QStringLiteral("com.apple.product-type.kernel-extension.iokit");
    //QStringLiteral("com.apple.product-type.kernel-extension.iokit.shallow");
    //QStringLiteral("com.apple.product-type.library.java.archive");
    //QStringLiteral("com.apple.product-type.objfile");
    //QStringLiteral("com.apple.product-type.pluginkit-plugin");
    //QStringLiteral("com.apple.product-type.spotlight-importer");
    //QStringLiteral("com.apple.product-type.tool.java");
}

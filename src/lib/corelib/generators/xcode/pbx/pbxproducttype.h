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

#ifndef PBXPRODUCTTYPE_H
#define PBXPRODUCTTYPE_H

#include <QString>

namespace PBX {
    typedef QString PBXProductType;

    // These have dedicated classes in Xcode
    extern const PBXProductType PBXApplicationProductType;
    extern const PBXProductType PBXBundleProductType;
    extern const PBXProductType PBXDynamicLibraryProductType;
    extern const PBXProductType PBXFrameworkProductType;
    extern const PBXProductType PBXShallowBundleProductType;
    extern const PBXProductType PBXShallowFrameworkProductType;
    extern const PBXProductType PBXStaticLibraryProductType;
    extern const PBXProductType PBXToolProductType;
    extern const PBXProductType XCKernelExtensionProductType;
    extern const PBXProductType XCShallowKernelExtensionProductType;
    extern const PBXProductType XCStaticFrameworkProductType;

    extern const PBXProductType XCApplicationExtensionProductType;
    extern const PBXProductType XCInAppPurchaseContentProductType;
    extern const PBXProductType XCXPCServiceProductType;
}

#endif // PBXPRODUCTTYPE_H

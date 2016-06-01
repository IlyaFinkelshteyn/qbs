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

#ifndef PBXFILEENCODING_H
#define PBXFILEENCODING_H

namespace PBX
{
    typedef enum {
        Default = 0,
        UTF8 = 4,
        UTF16 = 10,
        UTF16_BE = 0x90000100,
        UTF16_LE = 0x94000100,
        Western = 30,
        Japanese = 0x80000001,
        TraditionalChinese = 0x80000002,
        Korean = 0x80000003,
        Arabic = 0x80000004,
        Hebrew = 0x80000005,
        Greek = 0x80000006,
        Cyrillic = 0x80000007,
        SimplifiedChinese = 0x80000019,
        CentralEuropean = 0x8000001D,
        Turkish = 0x80000023,
        Icelandic = 0x80000025
    } PBXFileEncoding;
}

#endif // PBXFILEENCODING_H

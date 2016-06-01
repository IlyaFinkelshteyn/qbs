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

#include "pbxfiletype.h"
#include <QFileInfo>
#include <QMap>

PBXFileType::PBXFileType(QObject *parent) :
    QObject(parent)
{
}

QString PBXFileType::typeForFileExtension(const QString &fileExtension)
{
    // From DevToolsCore.framework's Resources, see *.pbfilespec
    static const QMap<QString, QString> map {
        {QStringLiteral("a"), QStringLiteral("archive.ar")},
        {QStringLiteral("app"), QStringLiteral("wrapper.application")},
        {QStringLiteral("bundle"), QStringLiteral("wrapper.plug-in")},
        //{QStringLiteral("bundle"), QStringLiteral("compiled.mach-o.bundle")},
        {QStringLiteral("c"), QStringLiteral("sourcecode.c")},
        {QStringLiteral("C"), QStringLiteral("sourcecode.cpp.cpp")},
        {QStringLiteral("class"), QStringLiteral("compiled.javaclass")},
        {QStringLiteral("cc"), QStringLiteral("sourcecode.cpp.cpp")},
        {QStringLiteral("cpp"), QStringLiteral("sourcecode.cpp.cpp")},
        {QStringLiteral("css"), QStringLiteral("text.css")},
        {QStringLiteral("cxx"), QStringLiteral("sourcecode.cpp.cpp")},
        {QStringLiteral("c++"), QStringLiteral("sourcecode.cpp.cpp")},
        {QStringLiteral("dsym"), QStringLiteral("wrapper.dsym")},
        {QStringLiteral("dSYM"), QStringLiteral("wrapper.dsym")},
        {QStringLiteral("dylib"), QStringLiteral("compiled.mach-o.dylib")},
        {QStringLiteral("framework"), QStringLiteral("wrapper.framework")},
        {QStringLiteral("h"), QStringLiteral("sourcecode.c.h")},
        {QStringLiteral("H"), QStringLiteral("sourcecode.c.h")},
        {QStringLiteral("hpp"), QStringLiteral("sourcecode.c.h")},
        {QStringLiteral("htm"), QStringLiteral("text.html")},
        {QStringLiteral("html"), QStringLiteral("text.html")},
        {QStringLiteral("hxx"), QStringLiteral("sourcecode.c.h")},
        {QStringLiteral("h++"), QStringLiteral("sourcecode.c.h")},
        {QStringLiteral("iconset"), QStringLiteral("folder.iconset")},
        {QStringLiteral("icns"), QStringLiteral("image.icns")},
        {QStringLiteral("ico"), QStringLiteral("image.ico")},
        {QStringLiteral("jar"), QStringLiteral("archive.jar")},
        {QStringLiteral("java"), QStringLiteral("sourcecode.java")},
        {QStringLiteral("js"), QStringLiteral("sourcecode.javascript")},
        {QStringLiteral("m"), QStringLiteral("sourcecode.c.objc")},
        {QStringLiteral("mm"), QStringLiteral("sourcecode.cpp.objcpp")},
        {QStringLiteral("php"), QStringLiteral("text.script.php")},
        {QStringLiteral("pl"), QStringLiteral("text.script.perl")},
        {QStringLiteral("plist"), QStringLiteral("text.plist.xml")},
        {QStringLiteral("py"), QStringLiteral("text.script.python")},
        {QStringLiteral("rb"), QStringLiteral("text.script.ruby")},
        {QStringLiteral("s"), QStringLiteral("sourcecode.asm")},
        {QStringLiteral("sh"), QStringLiteral("text.script")},
        {QStringLiteral("storyboard"), QStringLiteral("file.storyboard")},
        {QStringLiteral("swift"), QStringLiteral("sourcecode.swift")},
        {QStringLiteral("xcassets"), QStringLiteral("folder.assetcatalog")},
        {QStringLiteral("xib"), QStringLiteral("file.xib")},
        {QStringLiteral("xml"), QStringLiteral("text.xml")},
        //{QStringLiteral(""), QStringLiteral("")},
    };
    if (map.contains(fileExtension))
        return map[fileExtension];
    return QStringLiteral("text");
}

QString PBXFileType::typeForFileInfo(const QFileInfo &fileInfo)
{
    if (fileInfo.fileName().endsWith(QStringLiteral("Info.plist")))
        return QStringLiteral("text.plist.info");
    return typeForFileExtension(fileInfo.suffix());
}

bool PBXFileType::isBuildableFileType(const QString &fileType)
{
    static const QStringList buildableFileTypes {
        QStringLiteral("sourcecode.c"),
        QStringLiteral("sourcecode.cpp.cpp"),
        QStringLiteral("sourcecode.c.objc"),
        QStringLiteral("sourcecode.cpp.objcpp"),
        QStringLiteral("sourcecode.asm"),
        QStringLiteral("sourcecode.swift"),
        QStringLiteral("file.xib"),
        QStringLiteral("file.storyboard"),
        QStringLiteral("folder.assetcatalog"),
        QStringLiteral("folder.iconset")
    };
    return buildableFileTypes.contains(fileType);
}

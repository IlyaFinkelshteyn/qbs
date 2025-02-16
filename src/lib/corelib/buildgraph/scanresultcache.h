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

#ifndef QBS_SCANRESULTCACHE_H
#define QBS_SCANRESULTCACHE_H

#include <language/filetags.h>

#include <QHash>
#include <QString>
#include <QVector>

namespace qbs {
namespace Internal {

class ScanResultCache
{
public:
    class Dependency
    {
    public:
        Dependency() : m_isClean(true) {}
        Dependency(const QString &filePath);

        QString filePath() const { return m_dirPath.isEmpty() ? m_fileName : m_dirPath + QLatin1Char('/') + m_fileName; }
        const QString &dirPath() const { return m_dirPath; }
        const QString &fileName() const { return m_fileName; }
        bool isClean() const { return m_isClean; }

    private:
        QString m_dirPath;
        QString m_fileName;
        bool m_isClean;
    };

    class Result
    {
    public:
        Result()
            : valid(false)
        {}

        QVector<Dependency> deps;
        FileTags additionalFileTags;
        bool valid;
    };

    Result value(const void* scanner, const QString &fileName) const;
    void insert(const void* scanner, const QString &fileName, const Result &value);
    void remove(const QString &fileName);

private:
    typedef QHash<const void*, QHash<QString, Result> > ScanResultCacheData;
    ScanResultCacheData m_data;
};

} // namespace qbs
} // namespace qbs

#endif // QBS_SCANRESULTCACHE_H

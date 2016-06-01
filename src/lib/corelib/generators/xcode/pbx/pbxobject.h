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

#ifndef PBXOBJECT_H
#define PBXOBJECT_H

#include <QObject>
#include <QString>
#include <QVariant>

class PBXObjectPrivate;

class PBXObjectIdentifier {
public:
    PBXObjectIdentifier() {
    }

    PBXObjectIdentifier(const PBXObjectIdentifier &other) {
        _key = other._key;
        _isa = other._isa;
        _comment = other._comment;
    }

    PBXObjectIdentifier(const QString &identifier, const QString &isa = QString(), const QString &comment = QString()) {
        _key = identifier;
        _isa = isa;
        _comment = comment;
    }

    QString toString() const {
        return identifier();
    }

    QString identifier() const {
        return _key;
    }

    QString isa() const {
        return _isa;
    }

    QString comment() const {
        return _comment;
    }

    bool operator==(const PBXObjectIdentifier &other) const {
        return _key == other._key && _isa == other._isa && _comment == other._comment;
    }

    bool operator<(const PBXObjectIdentifier &other) const {
        return _isa + _key < other._isa + other._key;
    }

private:
    QString _key;
    QString _isa;
    QString _comment;
};

Q_DECLARE_METATYPE(PBXObjectIdentifier)

inline bool qHash(const PBXObjectIdentifier &other) {
    return qHash(other.toString());
}

typedef QMap<PBXObjectIdentifier, QVariant> PBXObjectMap;
Q_DECLARE_METATYPE(PBXObjectMap)

class PBXObject : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PBXObject)
protected:
    explicit PBXObject(QObject *parent = 0);

public:
    virtual ~PBXObject();

    static QString createIdentifier(const QByteArray &hashData);

    PBXObjectIdentifier identifier() const;
    virtual QString isa() const = 0;
    virtual PBXObjectMap toMap() const;
    virtual QString comment() const;
    virtual QByteArray hashData() const;

private:
    PBXObjectPrivate *d;
};

#endif // PBXOBJECT_H

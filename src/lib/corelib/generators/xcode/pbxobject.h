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

    PBXObjectIdentifier(const QLatin1String &identifier, const QString &isa = QString(), const QString &comment = QString()) {
        _key = identifier;
        _isa = isa;
        _comment = comment;
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

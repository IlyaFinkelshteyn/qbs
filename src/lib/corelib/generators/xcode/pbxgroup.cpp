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

#include "pbxfilereference.h"
#include "pbxgroup.h"

class PBXGroupPrivate
{
public:
    QList<PBXReference *> children;
};

PBXGroup::PBXGroup(QObject *parent) :
    PBXReference(parent), d(new PBXGroupPrivate)
{
}

PBXGroup::~PBXGroup()
{
    delete d;
}

PBXGroup *PBXGroup::groupWithName(const QString &name, PBXGroup *parentGroup)
{
    PBXGroup *group = new PBXGroup();
    group->setName(name);

    if (parentGroup) {
        group->setParent(parentGroup);
        parentGroup->addItem(group);
    }

    return group;
}

QString PBXGroup::isa() const
{
    return QLatin1String("PBXGroup");
}

PBXObjectMap PBXGroup::toMap() const
{
    PBXObjectMap self = PBXReference::toMap();

    QVariantList childIdentifiers;
    foreach (PBXReference *reference, d->children)
        childIdentifiers += QVariant::fromValue(reference->identifier());

    self.insert(QLatin1String("children"), childIdentifiers);

    return self;
}

QList<PBXReference *> PBXGroup::children() const
{
    return d->children;
}

void PBXGroup::addItem(PBXReference *item)
{
    d->children.append(item);
}

void PBXGroup::insertItem(PBXReference *item, int index)
{
    d->children.insert(index, item);
}

void PBXGroup::removeItem(PBXReference *item)
{
    d->children.removeOne(item);
}

QList<PBXFileReference *> PBXGroup::addFiles(const QStringList &files, bool copy, bool createGroupsRecursively)
{
    QList<PBXFileReference *> fileRefs;
    foreach (const QString &filePath, files)
    {
        PBXFileReference *fileReference = new PBXFileReference(this);
        fileReference->setPath(filePath);
        fileReference->setName(filePath.mid(filePath.lastIndexOf(QLatin1Char('/')) + 1));
        d->children.append(fileReference);
        fileRefs.append(fileReference);
    }

    return fileRefs;
}

QString PBXGroup::comment() const
{
    return !path().isEmpty() ? path() : PBXReference::comment();
}

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

#include "buildgraphnode.h"

#include "buildgraphvisitor.h"
#include "projectbuilddata.h"
#include <language/language.h>
#include <logging/translator.h>
#include <tools/error.h>
#include <tools/qbsassert.h>

//#include <qglobal.h>

namespace qbs {
namespace Internal {

BuildGraphNode::BuildGraphNode() : buildState(Untouched)
{
}

BuildGraphNode::~BuildGraphNode()
{
    foreach (BuildGraphNode *p, parents)
        p->children.remove(this);
    foreach (BuildGraphNode *c, children)
        c->parents.remove(this);
}

void BuildGraphNode::onChildDisconnected(BuildGraphNode *child)
{
    Q_UNUSED(child);
}

void BuildGraphNode::acceptChildren(BuildGraphVisitor *visitor)
{
    foreach (BuildGraphNode *child, children)
        child->accept(visitor);
}

void BuildGraphNode::load(PersistentPool &pool)
{
    children.load(pool);
    // Parents must be updated after loading all nodes.
}

void BuildGraphNode::store(PersistentPool &pool) const
{
    children.store(pool);
    // Do not store parents to avoid recursion.
}

} // namespace Internal
} // namespace qbs

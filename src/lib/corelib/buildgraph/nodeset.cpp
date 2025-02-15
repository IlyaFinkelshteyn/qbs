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

#include "nodeset.h"

#include "artifact.h"
#include "rulenode.h"
#include <language/language.h>  // because of RulePtr
#include <tools/persistence.h>
#include <tools/qbsassert.h>

namespace qbs {
namespace Internal {

NodeSet::NodeSet()
    : d(new NodeSetData)
{
}

NodeSet &NodeSet::unite(const NodeSet &other)
{
    d->m_data.insert(other.begin(), other.end());
    return *this;
}

void NodeSet::remove(BuildGraphNode *node)
{
    d->m_data.erase(node);
}

void NodeSet::load(PersistentPool &pool)
{
    clear();
    int i;
    pool.stream() >> i;
    for (; --i >= 0;) {
        int t;
        pool.stream() >> t;
        BuildGraphNode *node = 0;
        switch (static_cast<BuildGraphNode::Type>(t)) {
        case BuildGraphNode::ArtifactNodeType:
            node = pool.idLoad<Artifact>();
            break;
        case BuildGraphNode::RuleNodeType:
            node = pool.idLoad<RuleNode>();
            break;
        }
        QBS_CHECK(node);
        insert(node);
    }
}

void NodeSet::store(PersistentPool &pool) const
{
    pool.stream() << count();
    for (NodeSet::const_iterator it = constBegin(); it != constEnd(); ++it) {
        pool.stream() << int((*it)->type());
        pool.store(*it);
    }
}

} // namespace Internal
} // namespace qbs

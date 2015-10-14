// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
// 
// ================================================================================================


#include "treeitem.h"
#include "cgt/assert.h"
#include <iostream>

namespace campvis {

    TreeItem::TreeItem(TreeItem* parent /*= 0*/)
        : _parent(parent)
    {
        if (_parent != 0)
            _parent->_children.append(this);
    }

    TreeItem::~TreeItem() {
        qDeleteAll(_children);
    }

    TreeItem* TreeItem::getParent() {
        return _parent;
    }

    TreeItem* TreeItem::getChild(int row) {
        return _children.value(row);
    }

    int TreeItem::getRow() {
        if (_parent)
            return _parent->_children.indexOf(const_cast<TreeItem*>(this));

        return 0;
    }

    int TreeItem::getChildCount() {
        return _children.count();
    }

    bool TreeItem::setData(int column, int role, const QVariant& value) const {
        return false;
    }

    void TreeItem::insertChild(int row, TreeItem* child) {
        cgtAssert(row < 0 || row > _children.size(), "Row index out of bounds!");

        _children.insert(row, child);
        child->_parent = this;
    }

    void TreeItem::removeChild(int row) {
        cgtAssert(row < 0 || row > _children.size(), "Row index out of bounds!");
        delete _children.takeAt(row);
    }

    void TreeItem::replaceChild(int row, TreeItem* child) {
        cgtAssert(row < getChildCount(), "Row out of bounds!");
        delete _children[row];
        _children[row] = child;
        child->_parent = this;
    }

    void TreeItem::clearChildren() {
        qDeleteAll(_children);
        _children.clear();
    }

    void TreeItem::dumpTree(TreeItem* t, int level /*= 0*/) {
        std::cout << std::string(level, ' ') << t << "\n";
        for (int i = 0; i < t->getChildCount(); ++i) {
            TreeItem::dumpTree(t->getChild(i), level + 1);
        }
    }

}
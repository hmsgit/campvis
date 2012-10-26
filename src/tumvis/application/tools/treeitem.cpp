#include "treeitem.h"

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

}
// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
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



#include "luatablewidget.h"

#include "cgt/assert.h"

#include "application/gui/qtdatahandle.h"
#include "core/datastructures/datacontainer.h"
#include "core/datastructures/abstractdata.h"
#include "core/datastructures/dataseries.h"
#include "core/datastructures/lightsourcedata.h"
#include "core/datastructures/facegeometry.h"
#include "core/datastructures/meshgeometry.h"
#include "core/datastructures/indexedmeshgeometry.h"
#include "core/datastructures/multiindexedgeometry.h"
#include "core/datastructures/imageseries.h"
#include "core/datastructures/imagerepresentationdisk.h"
#include "core/datastructures/imagerepresentationlocal.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"

#include "core/tools/stringutils.h"

#ifdef CAMPVIS_HAS_MODULE_COLUMBIA
#include "modules/columbia/datastructures/fiberdata.h"
#endif

#include "scripting/glue/luatable.h"
#include "scripting/glue/globalluatable.h"
#include "scripting/glue/metatableluatable.h"
#include "scripting/glue/regularluatable.h"
#include "scripting/glue/luavmstate.h"

#include <QHeaderView>
#include <QStringList>

namespace campvis {

    namespace {
        const int COLUMN_NAME = 0;
        const int COLUMN_TYPE = 1;
        const int COLUMN_VALUE = 2;

    }

// = TreeModel items ==============================================================================

    LuaTreeItem::LuaTreeItem(ModelStyle modelStyle, const std::string& name, int type, TreeItem* parent /*= nullptr*/) : TreeItem(parent)
        , _name(name)
        , _type(type) 
        , _modelStyle(modelStyle)
    {
    }

    QVariant LuaTreeItem::getData(int column, int role) const {
        switch (role) {
            case Qt::EditRole: // fallthrough
            case Qt::DisplayRole:
                if (column == COLUMN_NAME)
                    return QVariant(QString::fromStdString(_name));
                else if (column == COLUMN_TYPE)
                    return QVariant(QString::fromStdString(lua_typename(0, _type)));
                else if (column == COLUMN_VALUE)
                    return getValue();
                else
                    return QVariant();
            default:
                return QVariant();
        }
    }

    QString LuaTreeItem::getValue() const {
        return QString("");
    }

    LuaTreeRootItem::LuaTreeRootItem(TreeItem* parent /*= 0*/)
        : TreeItem(parent)
    {}

// ================================================================================================

    QVariant LuaTreeRootItem::getData(int column, int role) const {
        if (role == Qt::DisplayRole) {
            if (column == COLUMN_NAME)
                return QVariant(QString("Name"));
            else if (column == COLUMN_TYPE)
                return QVariant(QString("Data Type"));
            else if (column == COLUMN_VALUE)
                return QVariant(QString("Value"));
        }

        return QVariant();
    }

    LuaTreeRootItem::~LuaTreeRootItem() {
    }

// ================================================================================================

    LuaTreeItemLeaf::LuaTreeItemLeaf(ModelStyle modelStyle, std::shared_ptr<LuaTable> parentTable, const std::string& name, int type, TreeItem* parent)
        : LuaTreeItem(modelStyle, name, type, parent)
        , _parentTable(parentTable)
    {
        if (parentTable->hasMetatable(_name)) {
            new LuaTreeItemTable(_modelStyle, true, parentTable->getMetatable(name), name, LUA_TTABLE, this);
        }
    }

    LuaTreeItemLeaf::~LuaTreeItemLeaf() {
    }
    
    QString LuaTreeItemLeaf::getValue() const {
        QString toReturn;
        lua_State* L = _parentTable->getLuaVmState().rawState();
        LuaStateMutexType::scoped_lock lock(_parentTable->getLuaVmState().getMutex());
        if (_type == LUA_TSTRING || _type == LUA_TNUMBER) {

            _parentTable->pushField(_name);
            toReturn = QString(lua_tostring(L, -1));
            _parentTable->popRecursive();
        }
        else if (_type == LUA_TUSERDATA) {
            auto mt = _parentTable->getMetatable(_name);
            if (mt) {
                mt->pushField(".type");
                toReturn = "[" + QString(lua_tostring(L, -1)) + "]";
                mt->popRecursive();
            }
        }
        return toReturn;
    }

// ================================================================================================

    LuaTreeItemTable::LuaTreeItemTable(ModelStyle modelStyle, bool isMetatable, std::shared_ptr<LuaTable> thisTable, const std::string& name, int type, TreeItem* parent) 
        : LuaTreeItem(modelStyle, name, type, parent)
        , _thisTable(thisTable)
        , _isMetatable(isMetatable)
    {
        // this casting here is not really that beautiful, but does it's job:
        // somehow, we need to get the parent table to check for the metatable.
        auto ltit = dynamic_cast<LuaTreeItemTable*>(getParent());
        if (ltit && ltit->_thisTable->hasMetatable(_name)) {
            new LuaTreeItemTable(_modelStyle, true, ltit->_thisTable->getMetatable(name), name, LUA_TTABLE, this);
        }

        // fill the table with values depending on model style
        if (_modelStyle == FULL_MODEL) {
            auto& valueMap = thisTable->getValueMap();
            for (auto it = valueMap.cbegin(); it != valueMap.cend(); ++it) {
                const std::string& itemName = it->first;
                int luaType = it->second.luaType;

                if (itemName == "_G")
                    continue;

                LuaTreeItem* lti = nullptr;
                if (luaType == LUA_TTABLE) 
                    lti = new LuaTreeItemTable(_modelStyle, false, thisTable->getTable(itemName), itemName, luaType, this);
                else 
                    lti = new LuaTreeItemLeaf(_modelStyle, thisTable, itemName, luaType, this);
            }
        }
        else if (_modelStyle == COMPLETER_MODEL) {
            auto& valueMap = thisTable->getValueMap();
            if (! _isMetatable) {
                // for regular tables, just explore the whole table
                for (auto it = valueMap.cbegin(); it != valueMap.cend(); ++it) {
                    const std::string& itemName = it->first;
                    int luaType = it->second.luaType;

                    if (itemName == "_G")
                        continue;

                    LuaTreeItem* lti = nullptr;
                    if (luaType == LUA_TTABLE) 
                        lti = new LuaTreeItemTable(_modelStyle, false, thisTable->getTable(itemName), itemName, luaType, this);
                    else 
                        lti = new LuaTreeItemLeaf(_modelStyle, thisTable, itemName, luaType, this);
                }
            }
            else {
                // for metatables, just gather all instance methods
                auto fnTable = thisTable->getTable(".fn");
                if (fnTable) {
                    recursiveGatherSwigMethods(thisTable, this);
                }

                auto instanceTable = thisTable->getTable(".instance");
                if (instanceTable) {
                    recursiveGatherSwigMethods(instanceTable, this);
                }
            }
        }
    }

    LuaTreeItemTable::~LuaTreeItemTable() {

    }

    QVariant LuaTreeItemTable::getData(int column, int role) const {
        if (_isMetatable && column == COLUMN_NAME && (role == Qt::EditRole || role == Qt::DisplayRole)) {
            switch (_modelStyle) {
                case FULL_MODEL:
                    return QVariant("[Metatable]");
                case COMPLETER_MODEL:
                    return QVariant("[Methods]");
            }
        }
        else {
            return LuaTreeItem::getData(column, role);
        }
    }

    void LuaTreeItemTable::recursiveGatherSwigMethods(const std::shared_ptr<LuaTable>& baseTable, TreeItem* parent) {
        // first get functions
        auto fnTable = baseTable->getTable(".fn");
        if (fnTable) {
            auto& valueMap = fnTable->getValueMap();
            for (auto it = valueMap.cbegin(); it != valueMap.cend(); ++it) {
                const std::string& itemName = it->first;
                int luaType = it->second.luaType;
                if (luaType == LUA_TFUNCTION && itemName.substr(0, 2) != "__") 
                    new LuaTreeItemLeaf(_modelStyle, fnTable, itemName, luaType, parent);
            }
        }

        // now walk through base classes and recursively gather their methods
        auto basesTable = baseTable->getTable(".bases");
        if (basesTable) {
            auto& valueMap = basesTable->getValueMap();
            for (auto it = valueMap.cbegin(); it != valueMap.cend(); ++it) {
                const std::string& itemName = it->first;
                int luaType = it->second.luaType;

                if (luaType == LUA_TTABLE)
                    recursiveGatherSwigMethods(basesTable->getTable(itemName), parent);
            }
        }
    }

    // = LuaTableTreeModel ============================================================================

    LuaTableTreeModel::LuaTableTreeModel(QObject *parent /*= 0*/)
        : QAbstractItemModel(parent)
        , _rootItem(new LuaTreeRootItem(0))
    {
    }

    LuaTableTreeModel::~LuaTableTreeModel() {
        delete _rootItem;
    }

    QVariant LuaTableTreeModel::data(const QModelIndex &index, int role) const {
        if (!index.isValid())
            return QVariant();

        TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
        return item->getData(index.column(), role);
    }

    bool LuaTableTreeModel::setData(const QModelIndex& index, const QVariant& value, int role) {
        if (!index.isValid())
            return false;

        TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
        return item->setData(index.column(), role, value);
    }

    Qt::ItemFlags LuaTableTreeModel::flags(const QModelIndex &index) const {
        if (!index.isValid())
            return 0;

        switch (index.column()) {
            case COLUMN_TYPE:
                return QAbstractItemModel::flags(index) | Qt::ItemIsSelectable;
            case COLUMN_NAME:
                return QAbstractItemModel::flags(index) | Qt::ItemIsSelectable;
            case COLUMN_VALUE:
                return QAbstractItemModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        }

        return 0;
    }

    QVariant LuaTableTreeModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
            return _rootItem->getData(section, role);

        return QVariant();
    }

    QModelIndex LuaTableTreeModel::index(int row, int column, const QModelIndex &parent /*= QModelIndex()*/) const {
        if (!hasIndex(row, column, parent))
            return QModelIndex();

        TreeItem* parentItem;

        if (!parent.isValid())
            parentItem = _rootItem;
        else
            parentItem = static_cast<TreeItem*>(parent.internalPointer());

        TreeItem* childItem = parentItem->getChild(row);
        if (childItem)
            return createIndex(row, column, childItem);
        else
            return QModelIndex();
    }

    QModelIndex LuaTableTreeModel::parent(const QModelIndex &index) const {
        if (!index.isValid())
            return QModelIndex();

        TreeItem* childItem = static_cast<TreeItem*>(index.internalPointer());
        TreeItem* parentItem = childItem->getParent();

        if (parentItem == _rootItem)
            return QModelIndex();

        return createIndex(parentItem->getRow(), 0, parentItem);
    }

    int LuaTableTreeModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const {
        TreeItem* parentItem;
        if (parent.column() > 0)
            return 0;

        if (!parent.isValid())
            parentItem = _rootItem;
        else
            parentItem = static_cast<TreeItem*>(parent.internalPointer());

        return parentItem->getChildCount();
    }

    int LuaTableTreeModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const {
        return 3;
    }

    void LuaTableTreeModel::setData(LuaVmState* luaVmState, LuaTreeItem::ModelStyle modelStyle) {
        beginResetModel();

        _itemMap.clear();
        delete _rootItem;
        _rootItem = new LuaTreeRootItem();

        if (luaVmState)
            new LuaTreeItemTable(modelStyle, false, luaVmState->getGlobalTable(), "[Global Variables]", LUA_TTABLE, _rootItem);

        endResetModel();
    }
    
// = LuaTableTreeWidget ===========================================================================

    LuaTableTreeWidget::LuaTableTreeWidget(QWidget* parent /*= 0*/)
        : QTreeView(parent)
    {
        setupWidget();
    }

    LuaTableTreeWidget::~LuaTableTreeWidget() {

    }

    void LuaTableTreeWidget::update(LuaVmState* luaVmState, LuaTreeItem::ModelStyle modelStyle) {
        // clear selection before setting the new data or we will encounter random crashes...
        selectionModel()->clear();

        // set new data
        _treeModel->setData(luaVmState, modelStyle);
        expandToDepth(0);

        // adjust view
        resizeColumnToContents(0);
        resizeColumnToContents(1);
        resizeColumnToContents(2);
    }

    void LuaTableTreeWidget::setupWidget() {
        _treeModel = new LuaTableTreeModel(this);
        cgtAssert(_treeModel != 0, "Failed creating TreeViewWidget model.");

        setModel(_treeModel);
    }

    LuaTableTreeModel* LuaTableTreeWidget::getTreeModel() {
        return _treeModel;
    }


}

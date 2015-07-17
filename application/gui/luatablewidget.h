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

#ifndef LUATABLEWIDGET_H__
#define LUATABLEWIDGET_H__

#include <QAbstractItemModel>
#include <QTreeWidget>
#include <QList>
#include <QVariant>
#include <map>

#include "application/applicationapi.h"
#include "application/tools/treeitem.h"
#include "application/gui/qtdatahandle.h"

#include "scripting/glue/luavmstate.h"
#include "scripting/glue/luatable.h"

namespace campvis {
    class DataContainer;

// = TreeModel items ==============================================================================
    
    /// Base class for LuaTableTreeWidget items
    class CAMPVIS_APPLICATION_API LuaTreeItem : public TreeItem {
    public:
        enum ModelStyle { FULL_MODEL, COMPLETER_MODEL };

        LuaTreeItem(ModelStyle modelStyle, const std::string& name, int type, TreeItem* parent = nullptr);

        // Virtual Destructor
        virtual ~LuaTreeItem() {}

        /// \see TreeItem::getData()
        virtual QVariant getData(int column, int role) const;

    protected:
        std::string _name;          ///< Name of the variable
        int _type;                  ///< Lua type of the variable
        ModelStyle _modelStyle;     ///< Model style for this tree item

    private:
        virtual QString getValue() const;
    };

    /// The Root Item
    class CAMPVIS_APPLICATION_API LuaTreeRootItem : public TreeItem {
    public:
        LuaTreeRootItem(TreeItem* parent = 0);
        virtual ~LuaTreeRootItem();

        /// \see TreeItem::getData()
        virtual QVariant getData(int column, int role) const;
    };

    /// Specialization for normal leafs
    class CAMPVIS_APPLICATION_API LuaTreeItemLeaf : public LuaTreeItem {
    public:
        /**
         * Creates a new TreeItem for a QtDataHandle
         * \param   name        Name of the Lua variable
         * \param   type        Lua type of the variable
         * \param   parent      Parent TreeItem
         */
        LuaTreeItemLeaf(ModelStyle modelStyle, std::shared_ptr<LuaTable> parentTable, const std::string& name, int type, TreeItem* parent);

        /// Destructor
        virtual ~LuaTreeItemLeaf();

    private:
        std::shared_ptr<LuaTable> _parentTable;    ///< this item's parent LuaTable
        virtual QString getValue() const;
    };
    
    /// Specialization for normal table items
    class CAMPVIS_APPLICATION_API LuaTreeItemTable : public LuaTreeItem {
    public:
        /**
         * Creates a new TreeItem for a QtDataHandle
         * \param   name        Name of the Lua variable
         * \param   type        Lua type of the variable
         * \param   parent      Parent TreeItem
         */
        LuaTreeItemTable(ModelStyle modelStyle, bool isMetatable, std::shared_ptr<LuaTable> thisTable, const std::string& name, int type, TreeItem* parent);

        /// Destructor
        virtual ~LuaTreeItemTable();

        /// \see TreeItem::getData()
        virtual QVariant getData(int column, int role) const;

    private:
        void recursiveGatherSwigMethods(const std::shared_ptr<LuaTable>& baseTable, TreeItem* parent);

        std::shared_ptr<LuaTable> _thisTable;    ///< this item's LuaTable
        bool _isMetatable;                      ///< Flag whether this item represents a Metatable (currently only used for printing purposes)
    };

// = TreeModel ====================================================================================

    /**
     * QItemModel for displaying a list of pipelines and their processors in the LuaTableTreeWidget.
     */
    class CAMPVIS_APPLICATION_API LuaTableTreeModel : public QAbstractItemModel {
        Q_OBJECT

    public:
        explicit LuaTableTreeModel(QObject *parent = 0);
        ~LuaTableTreeModel();

        void setData(LuaVmState* luaVmState, LuaTreeItem::ModelStyle modelStyle);

        QVariant data(const QModelIndex &index, int role) const;

        /**
         * Sets the data at index \a index to \a value.
         * \param   index   position where to set data
         * \param   value   new data
         * \param   role    edit role
         **/
        bool setData(const QModelIndex& index, const QVariant& value, int role);

        Qt::ItemFlags flags(const QModelIndex &index) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
        QModelIndex parent(const QModelIndex &index) const;

        int rowCount(const QModelIndex &parent = QModelIndex()) const;
        int columnCount(const QModelIndex &parent = QModelIndex()) const;

    private:
        TreeItem* _rootItem;
        std::map<QString, LuaTreeItemLeaf*> _itemMap;  ///< Mapping the QtDataHandle Keys to the TreeItems
    };

// = Widget =======================================================================================

    /**
     * Qt widget for showing a list of pipelines and their processors in a QTreeView.
     */
    class CAMPVIS_APPLICATION_API LuaTableTreeWidget : public QTreeView {
        Q_OBJECT;

    public:
        /**
         * Creates a new LuaTableTreeWidget.
         * \param   parent  Parent widget
         */
        explicit LuaTableTreeWidget(QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~LuaTableTreeWidget();

        /**
         * Returns the data model for the TreeView.
         * \return  _treeModel
         */
        LuaTableTreeModel* getTreeModel();


    public slots:
        /**
         * Updates the data in the tree view by the given collection of pipelines \a pipelines.
         * \param   pipelines   
         */
        void update(LuaVmState* luaVmState, LuaTreeItem::ModelStyle modelStyle);



    private:
        /**
         * Sets up the widget.
         */
        void setupWidget();

        LuaTableTreeModel* _treeModel;     ///< Data model for the TreeView.

    };

}

#endif // LUATABLEWIDGET_H__

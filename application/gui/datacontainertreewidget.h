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

#ifndef DATACONTAINERTREEWIDGET_H__
#define DATACONTAINERTREEWIDGET_H__

#include <QAbstractItemModel>
#include <QTreeWidget>
#include <QList>
#include <QVariant>
#include <map>

#include "application/applicationapi.h"
#include "application/tools/treeitem.h"
#include "application/gui/qtdatahandle.h"

namespace campvis {
    class DataContainer;

// = TreeModel items ==============================================================================
    
    /**
     * Specialization for root TreeItems.
     */
    class CAMPVIS_APPLICATION_API DataContainerTreeRootItem : public TreeItem {
    public:
        explicit DataContainerTreeRootItem(TreeItem* parent = 0);
        virtual ~DataContainerTreeRootItem();

        /// \see TreeItem::getData()
        virtual QVariant getData(int column, int role) const;
    };

    /**
     * Specialization for TreeItems hosting an AbstractProcessor.
     */
    class CAMPVIS_APPLICATION_API DataHandleTreeItem : public TreeItem {
    public:
        /**
         * Creates a new TreeItem for a QtDataHandle
         * \param   dataHandle  The QtDataHandle to wrap around, DataHandleTreeItem takes ownership of this pointer!
         * \param   name        Name of the QtDataHandle
         * \param   parent      Parent TreeItem
         */
        DataHandleTreeItem(QtDataHandle dataHandle, const std::string& name, TreeItem* parent);

        /**
         * Destructor, deletes the QtDataHandle
         */
        virtual ~DataHandleTreeItem();

        /// \see TreeItem::getData()
        virtual QVariant getData(int column, int role) const;

        /**
         * Sets the QtDataHandle for this tree item.
         * \note    DataHandleTreeItem takes ownership of this pointer.
         * \param   dataHandle  The QtDataHandle to wrap around, DataHandleTreeItem takes ownership of this pointer!
         */
        void setDataHandle(QtDataHandle dataHandle);

    private:
        /**
         * Updates this item's children.
         * In case of having a ImageSeries or RenderData as DataHandle, we are a collection of 
         * DataHandles ourself. Hence, we create child items for us.
         */
        void updateChildren();

        QtDataHandle _dataHandle;         ///< Base QtDataHandle
        std::string _name;              ///< Name of that QtDataHandle
    };

// = TreeModel ====================================================================================

    /**
     * QItemModel for displaying a list of pipelines and their processors in the DataContainerTreeWidget.
     */
    class CAMPVIS_APPLICATION_API DataContainerTreeModel : public QAbstractItemModel {
        Q_OBJECT

    public:
        explicit DataContainerTreeModel(QObject *parent = 0);
        ~DataContainerTreeModel();

        void setData(const DataContainer* dataContainer);

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

    public slots:
        /**
         * Slot being called when a QtDataHandle has been added to the DataContainer.
         * \param   key     Name of the QtDataHandle
         * \param   dh      The added QtDataHandle
         */
        void onDataContainerChanged(const QString& key, QtDataHandle dh);

    private:
        TreeItem* _rootItem;
        std::map<QString, DataHandleTreeItem*> _itemMap;  ///< Mapping the QtDataHandle Keys to the TreeItems
    };

// = Widget =======================================================================================

    /**
     * Qt widget for showing a list of pipelines and their processors in a QTreeView.
     */
    class CAMPVIS_APPLICATION_API DataContainerTreeWidget : public QTreeView {
        Q_OBJECT;

    public:
        /**
         * Creates a new DataContainerTreeWidget.
         * \param   parent  Parent widget
         */
        explicit DataContainerTreeWidget(QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~DataContainerTreeWidget();

        /**
         * Returns the data model for the TreeView.
         * \return  _treeModel
         */
        DataContainerTreeModel* getTreeModel();


    public slots:
        /**
         * Updates the data in the tree view by the given collection of pipelines \a pipelines.
         * \param   pipelines   
         */
        void update(const DataContainer* dataContainer);



    private:
        /**
         * Sets up the widget.
         */
        void setupWidget();

        DataContainerTreeModel* _treeModel;     ///< Data model for the TreeView.

    };

}

#endif // DATACONTAINERTREEWIDGET_H__

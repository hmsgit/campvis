// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#ifndef DATACONTAINERTREEWIDGET_H__
#define DATACONTAINERTREEWIDGET_H__

#include <QAbstractItemModel>
#include <QTreeWidget>
#include <QList>
#include <QVariant>
#include <map>

#include "application/tools/treeitem.h"
#include "application/gui/qtdatahandle.h"

namespace TUMVis {
    class DataContainer;

// = TreeModel items ==============================================================================
    
    /**
     * Specialization for root TreeItems.
     */
    class DataContainerTreeRootItem : public TreeItem {
    public:
        DataContainerTreeRootItem(TreeItem* parent = 0);
        virtual ~DataContainerTreeRootItem();

        /// \see TreeItem::getData()
        virtual QVariant getData(int column, int role) const;
    };

    /**
     * Specialization for TreeItems hosting an AbstractProcessor.
     */
    class DataHandleTreeItem : public TreeItem {
    public:
        /**
         * Creates a new TreeItem for a QtDataHandle
         * \param   dataHandle  The QtDataHandle to wrap around, DataHandleTreeItem takes ownership of this pointer!
         * \param   name        Name of the QtDataHandle
         * \param   parent      Parent TreeItem
         */
        DataHandleTreeItem(const QtDataHandle& dataHandle, const std::string& name, TreeItem* parent);

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
        void setDataHandle(const QtDataHandle& dataHandle);

    private:
        QtDataHandle _dataHandle;         ///< Base QtDataHandle
        std::string _name;              ///< Name of that QtDataHandle
    };

// = TreeModel ====================================================================================

    /**
     * QItemModel for displaying a list of pipelines and their processors in the DataContainerTreeWidget.
     */
    class DataContainerTreeModel : public QAbstractItemModel {
        Q_OBJECT

    public:
        DataContainerTreeModel(QObject *parent = 0);
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
    class DataContainerTreeWidget : public QTreeView {
        Q_OBJECT;

    public:
        /**
         * Creates a new DataContainerTreeWidget.
         * \param   parent  Parent widget
         */
        DataContainerTreeWidget(QWidget* parent = 0);

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

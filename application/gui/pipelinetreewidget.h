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

#ifndef PIPELINETREEWIDGET_H__
#define PIPELINETREEWIDGET_H__

#include <QAbstractItemModel>
#include <QTreeWidget>
#include <QList>
#include <QVariant>

#include "core/datastructures/datacontainer.h"
#include "core/pipeline/abstractpipeline.h"
#include "core/pipeline/abstractprocessor.h"

#include "application/applicationapi.h"
#include "application/tools/treeitem.h"

#include <vector>

namespace campvis {

// = TreeModel items ==============================================================================
    
    /**
     * Specialization for root TreeItems.
     */
    class CAMPVIS_APPLICATION_API PipelineTreeRootItem : public TreeItem {
    public:
        explicit PipelineTreeRootItem(TreeItem* parent = 0);
        virtual ~PipelineTreeRootItem();

        /// \see TreeItem::getData()
        virtual QVariant getData(int column, int role) const;
    };

    /**
     * Specialization for TreeItems hosting an AbstracPipeline.
     */
    class CAMPVIS_APPLICATION_API DataContainerTreeItem : public TreeItem {
    public:
        DataContainerTreeItem(DataContainer* dc, TreeItem* parent);
        virtual ~DataContainerTreeItem();

        /// \see TreeItem::getData()
        virtual QVariant getData(int column, int role) const;

        /// \see TreeItem::setData()
        virtual bool setData(int column, int role, const QVariant& value) const;

    private:
        DataContainer* _dataContainer;        ///< Base DataContainer
    };

    /**
     * Specialization for TreeItems hosting an AbstracPipeline.
     */
    class CAMPVIS_APPLICATION_API PipelineTreeItem : public TreeItem {
    public:
        PipelineTreeItem(AbstractPipeline* pipeline, TreeItem* parent);
        virtual ~PipelineTreeItem();

        /// \see TreeItem::getData()
        virtual QVariant getData(int column, int role) const;

        /// \see TreeItem::setData()
        virtual bool setData(int column, int role, const QVariant& value) const;

    private:
        AbstractPipeline* _pipeline;        ///< Base pipeline
    };

    /**
     * Specialization for TreeItems hosting an AbstractProcessor.
     */
    class CAMPVIS_APPLICATION_API ProcessorTreeItem : public TreeItem {
    public:
        ProcessorTreeItem(AbstractProcessor* processor, TreeItem* parent);
        virtual ~ProcessorTreeItem();

        /// \see TreeItem::getData()
        virtual QVariant getData(int column, int role) const;

        /// \see TreeItem::setData()
        virtual bool setData(int column, int role, const QVariant& value) const;

    private:
        AbstractProcessor* _processor;      ///< Base processor
    };

// = TreeModel ====================================================================================

    /**
     * QItemModel for displaying a list of pipelines and their processors in the PipelineTreeWidget.
     */
    class CAMPVIS_APPLICATION_API PipelineTreeModel : public QAbstractItemModel {
        Q_OBJECT

    public:
        explicit PipelineTreeModel(QObject *parent = 0);
        ~PipelineTreeModel();

        void setData(const std::vector<DataContainer*>& dataContainers, const std::vector<AbstractPipeline*>& pipelines);

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
    };

// = Widget =======================================================================================

    /**
     * Qt widget for showing a list of pipelines and their processors in a QTreeView.
     */
    class CAMPVIS_APPLICATION_API PipelineTreeWidget : public QTreeView {
        Q_OBJECT;

    public:
        /**
         * Creates a new PipelineTreeWidget.
         * \param   parent  Parent widget
         */
        explicit PipelineTreeWidget(QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~PipelineTreeWidget();

        /**
         * Return the recommended size for the widget.
         *
         * This method tries to calculate a sensible height for the wigdet, while still reusing QTreeView's hardcoded
         * width.
         */
        virtual QSize sizeHint() const;


    signals:
        /// Emitted whenever the currently selected item changes
        void itemChanged(const QModelIndex& index);


    public slots:
        /**
         * Updates the data in the tree view by the given collection of pipelines \a pipelines.
         * \param   pipelines   
         */
        void update(const std::vector<DataContainer*>& dataContainers, const std::vector<AbstractPipeline*>& pipelines);


    protected:

        virtual void selectionChanged(const QItemSelection& selected, const QItemSelection &deselected);

    private:
        /**
         * Sets up the widget.
         */
        void setupWidget();

        PipelineTreeModel* _treeModel;      ///< Data model for the TreeView.

    };
}

#endif // PIPELINETREEWIDGET_H__

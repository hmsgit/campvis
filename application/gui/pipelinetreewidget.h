// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
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

#ifndef PIPELINETREEWIDGET_H__
#define PIPELINETREEWIDGET_H__

#include <QAbstractItemModel>
#include <QTreeWidget>
#include <QList>
#include <QVariant>

#include "core/pipeline/abstractpipeline.h"
#include "core/pipeline/abstractprocessor.h"
#include "application/tools/treeitem.h"
#include <vector>

namespace campvis {

// = TreeModel items ==============================================================================
    
    /**
     * Specialization for root TreeItems.
     */
    class PipelineTreeRootItem : public TreeItem {
    public:
        PipelineTreeRootItem(TreeItem* parent = 0);
        virtual ~PipelineTreeRootItem();

        /// \see TreeItem::getData()
        virtual QVariant getData(int column, int role) const;
    };

    /**
     * Specialization for TreeItems hosting an AbstracPipeline.
     */
    class PipelineTreeItem : public TreeItem {
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
    class ProcessorTreeItem : public TreeItem {
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
    class PipelineTreeModel : public QAbstractItemModel {
        Q_OBJECT

    public:
        PipelineTreeModel(QObject *parent = 0);
        ~PipelineTreeModel();

        void setData(const std::vector<AbstractPipeline*>& pipelines);

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
    class PipelineTreeWidget : public QTreeView {
        Q_OBJECT;

    public:
        /**
         * Creates a new PipelineTreeWidget.
         * \param   parent  Parent widget
         */
        PipelineTreeWidget(QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~PipelineTreeWidget();


    public slots:
        /**
         * Updates the data in the tree view by the given collection of pipelines \a pipelines.
         * \param   pipelines   
         */
        void update(const std::vector<AbstractPipeline*>& pipelines);


    private:
        /**
         * Sets up the widget.
         */
        void setupWidget();

        PipelineTreeModel* _treeModel;      ///< Data model for the TreeView.

    };
}

#endif // PIPELINETREEWIDGET_H__

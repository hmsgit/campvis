#ifndef PIPELINETREEWIDGET_H__
#define PIPELINETREEWIDGET_H__

#include <QAbstractItemModel>
#include <QTreeWidget>
#include <QList>
#include <QVariant>

#include "core/pipeline/abstractpipeline.h"
#include "core/pipeline/abstractprocessor.h"
#include <vector>

namespace TUMVis {

// = TreeModel items ==============================================================================

    /**
     * Abstract base class for TreeView items of the PipelineTreeWidget.
     */
    class TreeItem {
    public:
        /**
         * Creates a new TreeItem
         * \param   parent  Parent item, will take ownership of this TreeItem.
         */
        TreeItem(TreeItem* parent = 0);

        /**
         * Destuctor, also deletes all child items.
         */
        virtual ~TreeItem();

        /**
         * Return the of this item at the column \a column.
         * To be overwritten by subclasses.
         * \param   column  Column
         * \return  The data of this item at \a column.
         */
        virtual QVariant getData(int column) const = 0;

        /**
         * Returns the parent TreeItem
         * \return _parent
         */
        TreeItem* getParent();

        /**
         * Returns the \a row-th child TreeItem.
         * \param   row     Number of the child to return.
         * \return  _children[row]
         */
        TreeItem* getChild(int row);

        /**
         * Returns the row of this TreeItem.
         * \return _parent._children.indexOf(this)
         */
        int getRow();

        /**
         * Returns the number of children.
         * \return  _children.size();
         */
        int getChildCount();

    private:
        TreeItem* _parent;              ///< Parent TreeItem.
        QList<TreeItem*> _children;     ///< Collection of all child items.
    };

    /**
     * Specialization for root TreeItems.
     */
    class RootTreeItem : public TreeItem {
    public:
        RootTreeItem(TreeItem* parent = 0);
        virtual ~RootTreeItem();

        /// \see TreeItem::getData()
        virtual QVariant getData(int column) const;
    };

    /**
     * Specialization for TreeItems hosting an AbstracPipeline.
     */
    class PipelineTreeItem : public TreeItem {
    public:
        PipelineTreeItem(AbstractPipeline* pipeline, TreeItem* parent);
        virtual ~PipelineTreeItem();

        /// \see TreeItem::getData()
        virtual QVariant getData(int column) const;

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
        virtual QVariant getData(int column) const;

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

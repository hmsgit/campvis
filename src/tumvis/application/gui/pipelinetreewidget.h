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

    class TreeItem {
    public:
        TreeItem(TreeItem* parent = 0);
        virtual ~TreeItem();

        virtual QVariant getData(int column) const = 0;

        TreeItem* getParent();
        TreeItem* getChild(int row);
        int getRow();
        int getChildCount();

    private:
        TreeItem* _parent;
        QList<TreeItem*> _children;
    };

    class RootTreeItem : public TreeItem {
    public:
        RootTreeItem(TreeItem* parent = 0);
        virtual ~RootTreeItem();

        virtual QVariant getData(int column) const;
    };

    class PipelineTreeItem : public TreeItem {
    public:
        PipelineTreeItem(AbstractPipeline* pipeline, TreeItem* parent);
        virtual ~PipelineTreeItem();

        virtual QVariant getData(int column) const;

    private:
        AbstractPipeline* _pipeline;
    };

    class ProcessorTreeItem : public TreeItem {
    public:
        ProcessorTreeItem(AbstractProcessor* processor, TreeItem* parent);
        virtual ~ProcessorTreeItem();

        virtual QVariant getData(int column) const;

    private:
        AbstractProcessor* _processor;
    };

// = TreeModel ====================================================================================

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

    class PipelineTreeWidget : public QTreeView {
        Q_OBJECT;

    public:
        PipelineTreeWidget(QWidget* parent = 0);

        virtual ~PipelineTreeWidget();


    public slots:
        void update(const std::vector<AbstractPipeline*>& pipelines);


    private:
        void setupWidget();

        PipelineTreeModel* _treeModel;


    };
}

#endif // PIPELINETREEWIDGET_H__

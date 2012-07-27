#include "pipelinetreewidget.h"

#include "tgt/assert.h"
#include <QHeaderView>
#include <QStringList>

namespace TUMVis {

// = TreeModel items ==============================================================================

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
    
    PipelineTreeItem::PipelineTreeItem(AbstractPipeline* pipeline, TreeItem* parent)
        : TreeItem(parent)
        , _pipeline(pipeline)
    {
        tgtAssert(_pipeline != 0, "Pipeline must not be 0.");
    }

    QVariant PipelineTreeItem::getData(int column, int role) const {
        switch (role) {
            case Qt::DisplayRole:
                if (column == 0)
                    return QVariant(QString::fromStdString(_pipeline->getName()));
            case Qt::UserRole:
                return qVariantFromValue(static_cast<void*>(_pipeline));
            default:
                return QVariant();
        }
    }

    PipelineTreeItem::~PipelineTreeItem() {
    }


    ProcessorTreeItem::ProcessorTreeItem(AbstractProcessor* processor, TreeItem* parent)
        : TreeItem(parent)
        , _processor(processor)
    {
        tgtAssert(_processor != 0, "Processor must not be 0.");
    }

    QVariant ProcessorTreeItem::getData(int column, int role) const {
        switch (role) {
            case Qt::DisplayRole:
                if (column == 0)
                    return QVariant(QString::fromStdString(_processor->getName()));
            case Qt::UserRole:
                return qVariantFromValue(static_cast<void*>(_processor));
            default:
                return QVariant();
        }
    }

    ProcessorTreeItem::~ProcessorTreeItem() {
    }

    RootTreeItem::RootTreeItem(TreeItem* parent /*= 0*/)
        : TreeItem(parent)
    {}

    QVariant RootTreeItem::getData(int column, int role) const {
        if (role == Qt::DisplayRole) {
            if (column == 0)
                return QVariant(QString("Pipeline/Processor"));
            else if (column == 1)
                return QVariant(QString("Description"));
        }

        return QVariant();
    }

    RootTreeItem::~RootTreeItem() {

    }

// = PipelineTreeModel ============================================================================

    PipelineTreeModel::PipelineTreeModel(QObject *parent /*= 0*/)
        : QAbstractItemModel(parent)
        , _rootItem(new RootTreeItem(0))
    {
    }

    PipelineTreeModel::~PipelineTreeModel() {
        delete _rootItem;
    }

    QVariant PipelineTreeModel::data(const QModelIndex &index, int role) const {
        if (!index.isValid())
            return QVariant();

        TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
        return item->getData(index.column(), role);
    }

    Qt::ItemFlags PipelineTreeModel::flags(const QModelIndex &index) const {
        if (!index.isValid())
            return 0;

        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    QVariant PipelineTreeModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
            return _rootItem->getData(section, role);

        return QVariant();
    }

    QModelIndex PipelineTreeModel::index(int row, int column, const QModelIndex &parent /*= QModelIndex()*/) const {
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

    QModelIndex PipelineTreeModel::parent(const QModelIndex &index) const {
        if (!index.isValid())
            return QModelIndex();

        TreeItem* childItem = static_cast<TreeItem*>(index.internalPointer());
        TreeItem* parentItem = childItem->getParent();

        if (parentItem == _rootItem)
            return QModelIndex();

        return createIndex(parentItem->getRow(), 0, parentItem);
    }

    int PipelineTreeModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const {
        TreeItem* parentItem;
        if (parent.column() > 0)
            return 0;

        if (!parent.isValid())
            parentItem = _rootItem;
        else
            parentItem = static_cast<TreeItem*>(parent.internalPointer());

        return parentItem->getChildCount();
    }

    int PipelineTreeModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const {
        return 2;
    }

    void PipelineTreeModel::setData(const std::vector<AbstractPipeline*>& pipelines) {
        for (std::vector<AbstractPipeline*>::const_iterator pipe = pipelines.begin(); pipe != pipelines.end(); ++pipe) {
            PipelineTreeItem* pipeti = new PipelineTreeItem(*pipe, _rootItem);

            for (std::vector<AbstractProcessor*>::const_iterator proc = (*pipe)->getProcessors().begin(); proc != (*pipe)->getProcessors().end(); ++proc) {
                ProcessorTreeItem* procti = new ProcessorTreeItem(*proc, pipeti);
            }
        }
    }


// = PipelineTreeWidget ===========================================================================

    PipelineTreeWidget::PipelineTreeWidget(QWidget* parent /*= 0*/)
        : QTreeView(parent)
    {
        setupWidget();
    }

    PipelineTreeWidget::~PipelineTreeWidget() {

    }

    void PipelineTreeWidget::update(const std::vector<AbstractPipeline*>& pipelines) {
        _treeModel->setData(pipelines);
    }

    void PipelineTreeWidget::setupWidget() {
        _treeModel = new PipelineTreeModel(this);
        tgtAssert(_treeModel != 0, "Failed creating TreeViewWidget model.");

        setModel(_treeModel);
        resizeColumnToContents(0);
   }

}
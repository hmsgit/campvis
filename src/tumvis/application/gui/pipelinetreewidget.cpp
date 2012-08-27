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

#include "pipelinetreewidget.h"

#include "tgt/assert.h"
#include <QHeaderView>
#include <QStringList>

namespace TUMVis {

    namespace {
        const int COLUMN_NAME = 0;
        const int COLUMN_ENABLED_STATE = 1;
        const int COLUMN_DESCRIPTION = 2;
    }

// = TreeModel items ==============================================================================

    PipelineTreeItem::PipelineTreeItem(AbstractPipeline* pipeline, TreeItem* parent)
        : TreeItem(parent)
        , _pipeline(pipeline)
    {
        tgtAssert(_pipeline != 0, "Pipeline must not be 0.");
    }

    QVariant PipelineTreeItem::getData(int column, int role) const {
        switch (role) {
            case Qt::DisplayRole:
                if (column == COLUMN_NAME)
                    return QVariant(QString::fromStdString(_pipeline->getName()));
                else
                    return QVariant();
            case Qt::CheckStateRole:
                if (column == COLUMN_ENABLED_STATE)
                    return _pipeline->getEnabled() ? QVariant(Qt::Checked) : QVariant(Qt::Unchecked);
                else
                    return QVariant();
            case Qt::UserRole:
                return qVariantFromValue(static_cast<void*>(_pipeline));
            default:
                return QVariant();
        }
    }

    PipelineTreeItem::~PipelineTreeItem() {
    }

    bool PipelineTreeItem::setData(int column, int role, const QVariant& value) const {
        if (column == COLUMN_ENABLED_STATE) {
            if (role == Qt::CheckStateRole) {
                _pipeline->setEnabled(value == Qt::Checked ? true : false);
                return true;
            }
        }
        return false;
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
                if (column == COLUMN_NAME)
                    return QVariant(QString::fromStdString(_processor->getName()));
                else if (column == COLUMN_DESCRIPTION)
                    return QVariant(QString::fromStdString(_processor->getDescription()));
                else
                    return QVariant();
            case Qt::CheckStateRole:
                if (column == COLUMN_ENABLED_STATE)
                    return _processor->getEnabled() ? QVariant(Qt::Checked) : QVariant(Qt::Unchecked);
                else
                    return QVariant();
            case Qt::UserRole:
                return qVariantFromValue(static_cast<void*>(_processor));
            default:
                return QVariant();
        }
    }

    ProcessorTreeItem::~ProcessorTreeItem() {
    }

    bool ProcessorTreeItem::setData(int column, int role, const QVariant& value) const {
        if (column == COLUMN_ENABLED_STATE) {
            if (role == Qt::CheckStateRole) {
                _processor->setEnabled(value == Qt::Checked ? true : false);
                return true;
            }
        }
        return false;
    }

    PipelineTreeRootItem::PipelineTreeRootItem(TreeItem* parent /*= 0*/)
        : TreeItem(parent)
    {}

    QVariant PipelineTreeRootItem::getData(int column, int role) const {
        if (role == Qt::DisplayRole) {
            if (column == COLUMN_NAME)
                return QVariant(QString("Pipeline/Processor"));
            else if (column == COLUMN_DESCRIPTION)
                return QVariant(QString("Description"));
            else if (column == COLUMN_ENABLED_STATE)
                return QVariant(QString("Enabled"));
        }

        return QVariant();
    }

    PipelineTreeRootItem::~PipelineTreeRootItem() {

    }

// = PipelineTreeModel ============================================================================

    PipelineTreeModel::PipelineTreeModel(QObject *parent /*= 0*/)
        : QAbstractItemModel(parent)
        , _rootItem(new PipelineTreeRootItem(0))
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

    bool PipelineTreeModel::setData(const QModelIndex& index, const QVariant& value, int role) {
        if (!index.isValid())
            return false;

        TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
        return item->setData(index.column(), role, value);
    }

    Qt::ItemFlags PipelineTreeModel::flags(const QModelIndex &index) const {
        if (!index.isValid())
            return 0;

        switch (index.column()) {
            case COLUMN_DESCRIPTION:
                return QAbstractItemModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
            case COLUMN_NAME:
                return QAbstractItemModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
            case COLUMN_ENABLED_STATE:
                return QAbstractItemModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
        }

        return 0;
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
        return 3;
    }

    void PipelineTreeModel::setData(const std::vector<AbstractPipeline*>& pipelines) {
        delete _rootItem;
        _rootItem = new PipelineTreeRootItem();

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
        expandAll();
        resizeColumnToContents(0);
        resizeColumnToContents(1);
    }

    void PipelineTreeWidget::setupWidget() {
        _treeModel = new PipelineTreeModel(this);
        tgtAssert(_treeModel != 0, "Failed creating TreeViewWidget model.");

        setModel(_treeModel);
   }

}
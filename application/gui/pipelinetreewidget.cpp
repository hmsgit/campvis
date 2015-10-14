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

#include "pipelinetreewidget.h"

#include "cgt/assert.h"
#include <QApplication>
#include <QHeaderView>
#include <QScrollBar>
#include <QStringList>

namespace campvis {

    namespace {
        const int COLUMN_NAME = 0;
        const int COLUMN_ENABLED_STATE = 1;
        const int COLUMN_CLOCK_STATE = 2;
    }

// = TreeModel items ==============================================================================

    DataContainerTreeItem::DataContainerTreeItem(DataContainer* dc, TreeItem* parent)
        : TreeItem(parent)
        , _dataContainer(dc)
    {
        cgtAssert(_dataContainer != 0, "Pipeline must not be 0.");
    }

    QVariant DataContainerTreeItem::getData(int column, int role) const {
        switch (role) {
        case Qt::DisplayRole:
            if (column == COLUMN_NAME)
                return QVariant(QString::fromStdString(_dataContainer->getName()));
            else
                return QVariant();
        case Qt::CheckStateRole:
            return QVariant();
        case Qt::UserRole:
            return QVariant();
        case Qt::BackgroundRole:
            return QVariant(QApplication::palette().alternateBase());
        default:
            return QVariant();
        }
    }

    DataContainerTreeItem::~DataContainerTreeItem() {
    }

    bool DataContainerTreeItem::setData(int column, int role, const QVariant& value) const {
        return false;
    }

    // ================================================================================================

    PipelineTreeItem::PipelineTreeItem(AbstractPipeline* pipeline, TreeItem* parent)
        : TreeItem(parent)
        , _pipeline(pipeline)
    {
        cgtAssert(_pipeline != 0, "Pipeline must not be 0.");
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
            case Qt::SizeHintRole:
                if (column == COLUMN_ENABLED_STATE || column == COLUMN_CLOCK_STATE)
                    return QVariant(QSize());
                else
                    return QVariant();
            case Qt::BackgroundRole:
                return QVariant(QApplication::palette().alternateBase());
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

// ================================================================================================

    ProcessorTreeItem::ProcessorTreeItem(AbstractProcessor* processor, TreeItem* parent)
        : TreeItem(parent)
        , _processor(processor)
    {
        cgtAssert(_processor != 0, "Processor must not be 0.");
    }

    QVariant ProcessorTreeItem::getData(int column, int role) const {
        switch (role) {
            case Qt::DisplayRole:
                if (column == COLUMN_NAME)
                    return QVariant(QString::fromStdString(_processor->getName()));
                else
                    return QVariant();
            case Qt::CheckStateRole:
                if (column == COLUMN_ENABLED_STATE)
                    return _processor->getEnabled() ? QVariant(Qt::Checked) : QVariant(Qt::Unchecked);
                else if (column == COLUMN_CLOCK_STATE)
                    return _processor->getClockExecutionTime() ? QVariant(Qt::Checked) : QVariant(Qt::Unchecked);
                else
                    return QVariant();
            case Qt::UserRole:
                return qVariantFromValue(static_cast<void*>(_processor));
            case Qt::ToolTipRole:
                return QVariant(QString::fromStdString(_processor->getDescription()));
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
        else if (column == COLUMN_CLOCK_STATE) {
            if (role == Qt::CheckStateRole) {
                _processor->setClockExecutionTime(value == Qt::Checked ? true : false);
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
            else if (column == COLUMN_ENABLED_STATE)
                return QVariant(QString("E"));
            else if (column == COLUMN_CLOCK_STATE)
                return QVariant(QString("P"));
        }
        else if (role == Qt::ToolTipRole) {
            if (column == COLUMN_NAME)
                return QVariant(QString("Name of Pipeline/Processor"));
            else if (column == COLUMN_ENABLED_STATE)
                return QVariant(QString("Enabled"));
            else if (column == COLUMN_CLOCK_STATE)
                return QVariant(QString("Profile Processor's execution time"));
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
            case COLUMN_NAME:
                return QAbstractItemModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
            case COLUMN_ENABLED_STATE:
                return QAbstractItemModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
            case COLUMN_CLOCK_STATE:
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

    void PipelineTreeModel::setData(const std::vector<DataContainer*>& dataContainers, const std::vector<AbstractPipeline*>& pipelines) {
        delete _rootItem;
        _rootItem = new PipelineTreeRootItem();

        std::map<DataContainer*, DataContainerTreeItem*> dcItemMap;
        for (size_t i = 0; i < dataContainers.size(); ++i) {
            DataContainerTreeItem* dcti = new DataContainerTreeItem(dataContainers[i], _rootItem);
            dcItemMap[dataContainers[i]] = dcti;
        }

        for (std::vector<AbstractPipeline*>::const_iterator pipe = pipelines.begin(); pipe != pipelines.end(); ++pipe) {
            std::map<DataContainer*, DataContainerTreeItem*>::iterator it = dcItemMap.find(&(*pipe)->getDataContainer());

            if (it != dcItemMap.end()) {
                PipelineTreeItem* pipeti = new PipelineTreeItem(*pipe, it->second);

                for (std::vector<AbstractProcessor*>::const_iterator proc = (*pipe)->getProcessors().begin(); proc != (*pipe)->getProcessors().end(); ++proc) {
                    new ProcessorTreeItem(*proc, pipeti);
                }
            }
            else {
                cgtAssert(false, "The DataContainer of this pipeline is not in the list of DataContainers, cannot add it to GUI!");
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

    QSize PipelineTreeWidget::sizeHint() const {
        int height = 0;

        // First of all, sum up the heights of top-level items and their children
        for (int i = 0; i < model()->rowCount(); i++) {
            QModelIndex index = model()->index(i, 0);
            height += sizeHintForRow(i);

            if (model()->hasChildren(index)) {
                for (int j = 0; j < model()->rowCount(index); ++j) {
                    QModelIndex cIndex = index.child(j, 0);
                    height += sizeHintForIndex(cIndex).height();

                    if (model()->hasChildren(cIndex)) {
                        height += model()->rowCount(cIndex) * sizeHintForIndex(cIndex.child(0, 0)).height();
                    }
                }                
            }
        }

        // Next, add the heights of the horizontal scrollbar, header, and frame
        height += horizontalScrollBar()->sizeHint().height();
        height += 2 * header()->sizeHint().height();
        height += 2 * frameWidth();

        return QSize(QTreeView::sizeHint().width(), std::max(height, 200));
    }

    void PipelineTreeWidget::update(const std::vector<DataContainer*>& dataContainers, const std::vector<AbstractPipeline*>& pipelines) {
        // clear selection before setting the new data or we will encounter random crashes...
        selectionModel()->clear();

        _treeModel->setData(dataContainers, pipelines);
        expandAll();
        resizeColumnToContents(0);
        resizeColumnToContents(1);
        resizeColumnToContents(2);

        // The widget's size hint might have changed, notify the layout
        updateGeometry();
    }

    void PipelineTreeWidget::selectionChanged(const QItemSelection& selected, const QItemSelection &deselected) {
        QTreeView::selectionChanged(selected, deselected);
        QModelIndexList selection = this->selectedIndexes();
        if (!selection.isEmpty())
            emit itemChanged(selection.first());
    }

    void PipelineTreeWidget::setupWidget() {
        _treeModel = new PipelineTreeModel(this);
        cgtAssert(_treeModel != 0, "Failed creating TreeViewWidget model.");

        setModel(_treeModel);
    }

}

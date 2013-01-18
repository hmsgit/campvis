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



#include "datacontainertreewidget.h"

#include "tgt/assert.h"

#include "application/gui/qtdatahandle.h"
#include "core/datastructures/datacontainer.h"
#include "core/datastructures/abstractdata.h"
#include "core/datastructures/imagerepresentationdisk.h"
#include "core/datastructures/imagerepresentationlocal.h"
#include "core/datastructures/imagerepresentationrendertarget.h"
#include "core/datastructures/imagerepresentationgl.h"

#include <QHeaderView>
#include <QStringList>

namespace campvis {

    namespace {
        const int COLUMN_NAME = 0;
        const int COLUMN_TYPE = 1;
    }

// = TreeModel items ==============================================================================


    DataContainerTreeRootItem::DataContainerTreeRootItem(TreeItem* parent /*= 0*/)
        : TreeItem(parent)
    {}

    QVariant DataContainerTreeRootItem::getData(int column, int role) const {
        if (role == Qt::DisplayRole) {
            if (column == COLUMN_NAME)
                return QVariant(QString("Name"));
            else if (column == COLUMN_TYPE)
                return QVariant(QString("Data Type"));
        }

        return QVariant();
    }

    DataContainerTreeRootItem::~DataContainerTreeRootItem() {

    }

// ================================================================================================

    DataHandleTreeItem::DataHandleTreeItem(const QtDataHandle& dataHandle, const std::string& name, TreeItem* parent)
        : TreeItem(parent)
        , _dataHandle(dataHandle)
        , _name(name)
    {
        tgtAssert(_dataHandle.getData() != 0, "WTF - QtDataHandle with empty data?");
    }

    QVariant DataHandleTreeItem::getData(int column, int role) const {
        switch (role) {
        case Qt::DisplayRole:
            if (column == COLUMN_NAME)
                return QVariant(QString::fromStdString(_name));
            else if (column == COLUMN_TYPE) {
                const AbstractData* data = _dataHandle.getData();
                tgtAssert(data != 0, "WTF - QtDataHandle with empty data?");
                if (const ImageRepresentationDisk* tester = dynamic_cast<const ImageRepresentationDisk*>(data)) {
                	return QVariant(QString("ImageData on disk"));
                }
                else if (const ImageRepresentationLocal* tester = dynamic_cast<const ImageRepresentationLocal*>(data)) {
                	return QVariant(QString("ImageData in local memory"));
                }
                else if (const ImageRepresentationGL* tester = dynamic_cast<const ImageRepresentationGL*>(data)) {
                    return QVariant(QString("ImageData in OpenGL texture."));
                }
                else if (const ImageRepresentationRenderTarget* tester = dynamic_cast<const ImageRepresentationRenderTarget*>(data)) {
                    return QVariant(QString("RenderTarget"));
                }
            }
            else
                return QVariant();
        case Qt::UserRole:
            return qVariantFromValue(_dataHandle);
        default:
            return QVariant();
        }
    }

    DataHandleTreeItem::~DataHandleTreeItem() {
    }

    void DataHandleTreeItem::setDataHandle(const QtDataHandle& dataHandle) {
        _dataHandle = dataHandle;
    }

// = DataContainerTreeModel ============================================================================

    DataContainerTreeModel::DataContainerTreeModel(QObject *parent /*= 0*/)
        : QAbstractItemModel(parent)
        , _rootItem(new DataContainerTreeRootItem(0))
    {
    }

    DataContainerTreeModel::~DataContainerTreeModel() {
        delete _rootItem;
    }

    QVariant DataContainerTreeModel::data(const QModelIndex &index, int role) const {
        if (!index.isValid())
            return QVariant();

        TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
        return item->getData(index.column(), role);
    }

    bool DataContainerTreeModel::setData(const QModelIndex& index, const QVariant& value, int role) {
        if (!index.isValid())
            return false;

        TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
        return item->setData(index.column(), role, value);
    }

    Qt::ItemFlags DataContainerTreeModel::flags(const QModelIndex &index) const {
        if (!index.isValid())
            return 0;

        switch (index.column()) {
            case COLUMN_TYPE:
                return QAbstractItemModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
            case COLUMN_NAME:
                return QAbstractItemModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        }

        return 0;
    }

    QVariant DataContainerTreeModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
            return _rootItem->getData(section, role);

        return QVariant();
    }

    QModelIndex DataContainerTreeModel::index(int row, int column, const QModelIndex &parent /*= QModelIndex()*/) const {
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

    QModelIndex DataContainerTreeModel::parent(const QModelIndex &index) const {
        if (!index.isValid())
            return QModelIndex();

        TreeItem* childItem = static_cast<TreeItem*>(index.internalPointer());
        TreeItem* parentItem = childItem->getParent();

        if (parentItem == _rootItem)
            return QModelIndex();

        return createIndex(parentItem->getRow(), 0, parentItem);
    }

    int DataContainerTreeModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const {
        TreeItem* parentItem;
        if (parent.column() > 0)
            return 0;

        if (!parent.isValid())
            parentItem = _rootItem;
        else
            parentItem = static_cast<TreeItem*>(parent.internalPointer());

        return parentItem->getChildCount();
    }

    int DataContainerTreeModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const {
        return 2;
    }

    void DataContainerTreeModel::setData(const DataContainer* dataContainer) {
        _itemMap.clear();
        delete _rootItem;
        _rootItem = new DataContainerTreeRootItem();

        std::vector< std::pair< std::string, DataHandle> > handlesCopy = dataContainer->getDataHandlesCopy();
        for (std::vector< std::pair< std::string, DataHandle> >::iterator it = handlesCopy.begin(); it != handlesCopy.end(); ++it) {
            DataHandleTreeItem* dhti = new DataHandleTreeItem(QtDataHandle(it->second), it->first, _rootItem);
            _itemMap.insert(std::make_pair(QString::fromStdString(it->first), dhti));
        }
    }

    void DataContainerTreeModel::onDataContainerChanged(const QString& key, QtDataHandle dh) {
        tgtAssert(dh.getData() != 0, "WTF - QtDataHandle with empty data?");

        std::map<QString, DataHandleTreeItem*>::const_iterator it = _itemMap.lower_bound(key);
        if (it == _itemMap.end() || it->first != key) {
            DataHandleTreeItem* dhti = new DataHandleTreeItem(dh, key.toStdString(), _rootItem);
            _itemMap.insert(std::make_pair(key, dhti));
            emit dataChanged(createIndex(dhti->getRow(), 0, dhti), createIndex(dhti->getRow(), columnCount(), dhti));
        }
        else {
            it->second->setDataHandle(dh);
            emit dataChanged(createIndex(it->second->getRow(), 0, it->second), createIndex(it->second->getRow(), columnCount(), it->second));
        }
    }

// = DataContainerTreeWidget ===========================================================================

    DataContainerTreeWidget::DataContainerTreeWidget(QWidget* parent /*= 0*/)
        : QTreeView(parent)
    {
        setupWidget();
    }

    DataContainerTreeWidget::~DataContainerTreeWidget() {

    }

    void DataContainerTreeWidget::update(const DataContainer* dataContainer) {
        // clear selection before setting the new data or we will encounter random crashes...
        selectionModel()->clear();

        // set new data
        _treeModel->setData(dataContainer);

        // adjust view
        expandAll();
        resizeColumnToContents(0);
        resizeColumnToContents(1);
    }

    void DataContainerTreeWidget::setupWidget() {
        _treeModel = new DataContainerTreeModel(this);
        tgtAssert(_treeModel != 0, "Failed creating TreeViewWidget model.");

        setModel(_treeModel);
    }

    DataContainerTreeModel* DataContainerTreeWidget::getTreeModel() {
        return _treeModel;
    }


}
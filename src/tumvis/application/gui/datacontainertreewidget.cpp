

#include "datacontainertreewidget.h"

#include "tgt/assert.h"

#include "core/datastructures/datacontainer.h"
#include "core/datastructures/datahandle.h"
#include "core/datastructures/abstractdata.h"
#include "core/datastructures/imagedatadisk.h"
#include "core/datastructures/imagedatalocal.h"
#include "core/datastructures/imagedatarendertarget.h"
#include "core/datastructures/imagedatagl.h"

#include <QHeaderView>
#include <QStringList>

namespace TUMVis {

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

    DataHandleTreeItem::DataHandleTreeItem(const DataHandle* dataHandle, const std::string& name, TreeItem* parent)
        : TreeItem(parent)
        , _dataHandle(dataHandle)
        , _name(name)
    {
        tgtAssert(_dataHandle != 0, "DataHandle must not be 0.");
    }

    QVariant DataHandleTreeItem::getData(int column, int role) const {
        switch (role) {
        case Qt::DisplayRole:
            if (column == COLUMN_NAME)
                return QVariant(QString::fromStdString(_name));
            else if (column == COLUMN_TYPE) {
                const AbstractData* data = _dataHandle->getData();
                tgtAssert(data != 0, "WTF - DataHandle with empty data?");
                if (const ImageDataDisk* tester = dynamic_cast<const ImageDataDisk*>(data)) {
                	return QVariant(QString("ImageData on disk"));
                }
                else if (const ImageDataLocal* tester = dynamic_cast<const ImageDataLocal*>(data)) {
                	return QVariant(QString("ImageData in local memory"));
                }
                else if (const ImageDataGL* tester = dynamic_cast<const ImageDataGL*>(data)) {
                    return QVariant(QString("ImageData in OpenGL texture."));
                }
                else if (const ImageDataRenderTarget* tester = dynamic_cast<const ImageDataRenderTarget*>(data)) {
                    return QVariant(QString("RenderTarget"));
                }
            }
            else
                return QVariant();
        case Qt::UserRole:
            // const cast ist ugly, same as GUI programming...
            return qVariantFromValue(const_cast<void*>(static_cast<const void*>(_dataHandle)));
        default:
            return QVariant();
        }
    }

    DataHandleTreeItem::~DataHandleTreeItem() {
        delete _dataHandle;
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
        delete _rootItem;
        _rootItem = new DataContainerTreeRootItem();

        std::vector< std::pair< std::string, const DataHandle*> > handlesCopy = dataContainer->getDataHandlesCopy();
        for (std::vector< std::pair< std::string, const DataHandle*> >::iterator it = handlesCopy.begin(); it != handlesCopy.end(); ++it) {
            DataHandleTreeItem* dhti = new DataHandleTreeItem(it->second, it->first, _rootItem);
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

}
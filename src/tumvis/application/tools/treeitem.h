#ifndef TREEITEM_H__
#define TREEITEM_H__

#include <QList>
#include <QVariant>

namespace TUMVis {

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
         * \param   role    Access role (Use Qt::DisplayRole for string representation, Qt::UserRole for the internal data)
         * \return  The data of this item at \a column.
         */
        virtual QVariant getData(int column, int role) const = 0;

        /**
         * Sets the data at the given column to \a value using the given role.
         * Overwrite if needed, default implementation will only return false.
         * \param   column  Column
         * \param   role    Access role
         * \param   value   Value to set
         * \return  false
         */
        virtual bool setData(int column, int role, const QVariant& value) const;

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

}

#endif // TREEITEM_H__
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

#ifndef TREEITEM_H__
#define TREEITEM_H__

#include "application/applicationapi.h"

#include <QList>
#include <QVariant>

namespace campvis {

    /**
     * Abstract base class for TreeView items of the PipelineTreeWidget.
     */
    class CAMPVIS_APPLICATION_API TreeItem {
    public:
        /**
         * Creates a new TreeItem
         * \param   parent  Parent item, will take ownership of this TreeItem.
         */
        explicit TreeItem(TreeItem* parent = 0);

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

        /**
         * Inserts a child at the given row.
         * \param   row     Row where to insert the child
         */
        void insertChild(int row, TreeItem* child);

        /**
         * Removes the child at the given row.
         * \param   row     Row index of the child to remove.
         */
        void removeChild(int row);

        /**
         * Replaces the child with index \a row with the given TreeItem.
         * \param   row     Child to replace, must be < than getChildCount()
         * \param   child   New TreeItem for the child
         */
        void replaceChild(int row, TreeItem* child);

        /**
         * Removes all children.
         */
        void clearChildren();

        /**
         * Dumps debug output of the tree hierarchy.
         * \param t     TreeItem to dump
         * \param level Starting indentation level
         */
        static void dumpTree(TreeItem* t, int level = 0);

    protected:
        TreeItem* _parent;              ///< Parent TreeItem.
        QList<TreeItem*> _children;     ///< Collection of all child items.
    };



}

#endif // TREEITEM_H__
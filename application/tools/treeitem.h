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

#ifndef TREEITEM_H__
#define TREEITEM_H__

#include <QList>
#include <QVariant>

namespace campvis {

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
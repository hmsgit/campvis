// ================================================================================================
// 
// This file is part of the KissCL, an OpenCL C++ wrapper following the KISS principle.
// 
// Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (software@cszb.net)
// 
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
// 
//  * Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright notice, this list of
//    conditions and the following disclaimer in the documentation and/or other materials provided
//    with the distribution.
//  * The names of its contributors may not be used to endorse or promote products derived from
//    this software without specific prior written permission.
//  
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
//  AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
//  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#ifndef KISSCL_ITEMLIST_H__
#define KISSCL_ITEMLIST_H__


namespace kisscl {

    /**
     * Wraps around the ids of a fixed list of OpenCL items.
     */
    template <typename T>
    struct ItemList {
        typedef typename T::cl_type ItemType;   ///< Typedef for the OpenCL id type of the item.

        /// Creates an empty ItemList
        explicit ItemList();

        /**
         * Creates an ItemList containing 1 items.
         * \param   i1   Item 1
         */
        explicit ItemList(const T& i1);

        /**
         * Creates an ItemList containing 2 items.
         * \param   i1   Item 1
         * \param   i2   Item 2
         */
        explicit ItemList(const T& i1, const T& i2);

        /**
         * Creates an ItemList containing 3 items.
         * \param   i1   Item 1
         * \param   i2   Item 2
         * \param   i3   Item 3
         */
        explicit ItemList(const T& i1, const T& i2, const T& i3);

        /**
         * Creates an ItemList containing 4 items.
         * \param   i1   Item 1
         * \param   i2   Item 2
         * \param   i3   Item 3
         * \param   i4   Item 4
         */
        explicit ItemList(const T& i1, const T& i2, const T& i3, const T& i4);

        /**
         * Creates an ItemList containing 5 items.
         * \param   i1   Item 1
         * \param   i2   Item 2
         * \param   i3   Item 3
         * \param   i4   Item 4
         * \param   i5   Item 5
         */
        explicit ItemList(const T& i1, const T& i2, const T& i3, const T& i4, const T& i5);

        /**
         * Creates an ItemList containing 6 items.
         * \param   i1   Item 1
         * \param   i2   Item 2
         * \param   i3   Item 3
         * \param   i4   Item 4
         * \param   i5   Item 5
         * \param   i6   Item 6
         */
        explicit ItemList(const T& i1, const T& i2, const T& i3, const T& i4, const T& i5, const T& i6);

        /**
         * Creates an ItemList containing 7 items.
         * \param   i1   Item 1
         * \param   i2   Item 2
         * \param   i3   Item 3
         * \param   i4   Item 4
         * \param   i5   Item 5
         * \param   i6   Item 6
         * \param   i7   Item 7
         */
        explicit ItemList(const T& i1, const T& i2, const T& i3, const T& i4, const T& i5, const T& i6, const T& i7);

        /**
         * Creates an ItemList containing 8 items.
         * \param   i1   Item 1
         * \param   i2   Item 2
         * \param   i3   Item 3
         * \param   i4   Item 4
         * \param   i5   Item 5
         * \param   i6   Item 6
         * \param   i7   Item 7
         * \param   i8   Item 8
         */
        explicit ItemList(const T& i1, const T& i2, const T& i3, const T& i4, const T& i5, const T& i6, const T& i7, const T& i8);

        /**
         * Creates an ItemList containing 9 items.
         * \param   i1   Item 1
         * \param   i2   Item 2
         * \param   i3   Item 3
         * \param   i4   Item 4
         * \param   i5   Item 5
         * \param   i6   Item 6
         * \param   i7   Item 7
         * \param   i8   Item 8
         * \param   i9   Item 9
         */
        explicit ItemList(const T& i1, const T& i2, const T& i3, const T& i4, const T& i5, const T& i6, const T& i7, const T& i8, const T& i9);

        /**
         * Creates an ItemList containing 10 items.
         * \param   i1   Item 1
         * \param   i2   Item 2
         * \param   i3   Item 3
         * \param   i4   Item 4
         * \param   i5   Item 5
         * \param   i6   Item 6
         * \param   i7   Item 7
         * \param   i8   Item 8
         * \param   i9   Item 9
         * \param   i10   Item 10
         */
        explicit ItemList(const T& i1, const T& i2, const T& i3, const T& i4, const T& i5, const T& i6, const T& i7, const T& i8, const T& i9, const T& i10);

        /// Destructor
        ~ItemList();

        size_t _size;           ///< Number of items in this list.
        ItemType* _items;       ///< Pointer to the Ids if the items.
    };

// ================================================================================================

    template <typename T>
    kisscl::ItemList<T>::ItemList()
        : _size(0)
        , _items(0)
    {
    }
    template <typename T>
    kisscl::ItemList<T>::ItemList(const T& i1)
        : _size(1)
        , _items(0)
    {
        _items = new ItemType[_size];
        _items[0] = i1.getId();
    }

    template <typename T>
    kisscl::ItemList<T>::ItemList(const T& i1, const T& i2)
        : _size(2)
        , _items(0)
    {
        _items = new ItemType[_size];
        _items[0] = i1.getId();
        _items[1] = i2.getId();
    }

    template <typename T>
    kisscl::ItemList<T>::ItemList(const T& i1, const T& i2, const T& i3)
        : _size(3)
        , _items(0)
    {
        _items = new ItemType[_size];
        _items[0] = i1.getId();
        _items[1] = i2.getId();
        _items[2] = i3.getId();
    }

    template <typename T>
    kisscl::ItemList<T>::ItemList(const T& i1, const T& i2, const T& i3, const T& i4)
        : _size(4)
        , _items(0)
    {
        _items = new ItemType[_size];
        _items[0] = i1.getId();
        _items[1] = i2.getId();
        _items[2] = i3.getId();
        _items[3] = i4.getId();
    }

    template <typename T>
    kisscl::ItemList<T>::ItemList(const T& i1, const T& i2, const T& i3, const T& i4, const T& i5)
        : _size(5)
        , _items(0)
    {
        _items = new ItemType[_size];
        _items[0] = i1.getId();
        _items[1] = i2.getId();
        _items[2] = i3.getId();
        _items[3] = i4.getId();
        _items[4] = i5.getId();
    }

    template <typename T>
    kisscl::ItemList<T>::ItemList(const T& i1, const T& i2, const T& i3, const T& i4, const T& i5, const T& i6)
        : _size(6)
        , _items(0)
    {
        _items = new ItemType[_size];
        _items[0] = i1.getId();
        _items[1] = i2.getId();
        _items[2] = i3.getId();
        _items[3] = i4.getId();
        _items[4] = i5.getId();
        _items[5] = i6.getId();
    }

    template <typename T>
    kisscl::ItemList<T>::ItemList(const T& i1, const T& i2, const T& i3, const T& i4, const T& i5, const T& i6, const T& i7)
        : _size(7)
        , _items(0)
    {
        _items = new ItemType[_size];
        _items[0] = i1.getId();
        _items[1] = i2.getId();
        _items[2] = i3.getId();
        _items[3] = i4.getId();
        _items[4] = i5.getId();
        _items[5] = i6.getId();
        _items[6] = i7.getId();
    }

    template <typename T>
    kisscl::ItemList<T>::ItemList(const T& i1, const T& i2, const T& i3, const T& i4, const T& i5, const T& i6, const T& i7, const T& i8)
        : _size(8)
        , _items(0)
    {
        _items = new ItemType[_size];
        _items[0] = i1.getId();
        _items[1] = i2.getId();
        _items[2] = i3.getId();
        _items[3] = i4.getId();
        _items[4] = i5.getId();
        _items[5] = i6.getId();
        _items[6] = i7.getId();
        _items[7] = i8.getId();
    }

    template <typename T>
    kisscl::ItemList<T>::ItemList(const T& i1, const T& i2, const T& i3, const T& i4, const T& i5, const T& i6, const T& i7, const T& i8, const T& i9)
        : _size(9)
        , _items(0)
    {
        _items = new ItemType[_size];
        _items[0] = i1.getId();
        _items[1] = i2.getId();
        _items[2] = i3.getId();
        _items[3] = i4.getId();
        _items[4] = i5.getId();
        _items[5] = i6.getId();
        _items[6] = i7.getId();
        _items[7] = i8.getId();
        _items[8] = i9.getId();
    }

    template <typename T>
    kisscl::ItemList<T>::ItemList(const T& i1, const T& i2, const T& i3, const T& i4, const T& i5, const T& i6, const T& i7, const T& i8, const T& i9, const T& i10)
        : _size(10)
        , _items(0)
    {
        _items = new ItemType[_size];
        _items[0] = i1.getId();
        _items[1] = i2.getId();
        _items[2] = i3.getId();
        _items[3] = i4.getId();
        _items[4] = i5.getId();
        _items[5] = i6.getId();
        _items[6] = i7.getId();
        _items[7] = i8.getId();
        _items[8] = i9.getId();
        _items[9] = i10.getId();
    }

    template <typename T>
    kisscl::ItemList<T>::~ItemList() {
        delete _items;
    }
}

#endif // KISSCL_ITEMLIST_H__

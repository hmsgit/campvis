/**********************************************************************
 *                                                                    *
 * cgt - CAMP Graphics Toolbox, Copyright (C) 2012-2014               *
 *     Chair for Computer Aided Medical Procedures                    *
 *     Technische Universitaet Muenchen, Germany.                     *
 *     <http://campar.in.tum.de/>                                     *
 *                                                                    *
 * forked from tgt - Tiny Graphics Toolbox, Copyright (C) 2006-2011   *
 *     Visualization and Computer Graphics Group, Department of       *
 *     Computer Science, University of Muenster, Germany.             *
 *     <http://viscg.uni-muenster.de>                                 *
 *                                                                    *
 * This file is part of the cgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#ifndef CGT_SINGLETON_H
#define CGT_SINGLETON_H

#include <iostream>

#include "cgt/assert.h"
#include "cgt/types.h"

namespace cgt {

/**
    This class helps to build the singleton design pattern.
    Here you have full control over construction and deconstruction
    of the object.
*/
template<class T>
class Singleton {
public:
    /**
     * Init the actual singleton.
     * Must be called BEFORE the class is used, like this:
     */
    static void init() {
        cgtAssert( !singletonClass_, "singletonClass_ has already been initialized." );
        singletonClass_ = new T;
    }

    /**
     * Deinit the actual singleton.
     * Must be done at last.
     */
    static void deinit() {
        cgtAssert( singletonClass_ != 0, "singletonClass_ has already been deinitialized." );
        delete singletonClass_;
        singletonClass_ = 0;
    }

    /**
     * Get Pointer of the actual class
     * @return Pointer of the actual class
     */
    static T* getPtr() {
        cgtAssert( singletonClass_ != 0, "singletonClass_ has not been intitialized." );
        return singletonClass_;
    }

    /**
     * Get reference of the actual class
     * @return reference of the actual class
    */
    static T& getRef() {
        cgtAssert( singletonClass_ != 0 , "singletonClass_ has not been intitialized." );
        return *singletonClass_;
    }

    /**
     * Has the actual class been inited?
     */
    static bool isInited() {
        return (singletonClass_ != 0);
    }

private:
    static T* singletonClass_;
};

/// init static pointers with 0
template<class T>
T* Singleton<T>::singletonClass_ = 0;

} // namespace

#endif // CGT_SINGLETON_H

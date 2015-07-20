/**********************************************************************
 *                                                                    *
 * cgt - CAMP Graphics Toolbox, Copyright (C) 2012-2015               *
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

#include "buffer.h"

namespace cgt {
    BufferObject::BufferObject(TargetType target, UsageType usage) throw (cgt::Exception)
        : _id(0)
        , _targetType(target)
        , _usageType(usage)
        , _baseType(BYTE)
        , _elementSize(1)
        , _size(0)
        , _numElements(0)
    {
        glGenBuffers(1, &_id);
        if (_id == 0) {
            throw new cgt::Exception("Could not create OpenGL Buffer object.");
        }
    }

    BufferObject::~BufferObject() {
        glDeleteBuffers(1, &_id);
    }

    void BufferObject::bind() {
        glBindBuffer(_targetType, _id);
    }

    void BufferObject::data(const void* data, size_t numBytes, BaseType baseType, size_t elementSize) {
        bind();
        glBufferData(_targetType, numBytes, data, _usageType);
        _baseType = baseType;
        _elementSize = elementSize;
    }

    void BufferObject::subdata(size_t offset, const void* data, size_t numBytes) {
        bind();
        glBufferSubData(_targetType, offset, numBytes, data);
    }

    BufferObject::BaseType BufferObject::getBaseType() const {
        return _baseType;
    }

    size_t BufferObject::getElementSize() const {
        return _elementSize;
    }

    size_t BufferObject::getBufferSize() const {
        return _size;
    }

    BufferObject::TargetType BufferObject::getTargetType() const {
        return _targetType;
    }

}
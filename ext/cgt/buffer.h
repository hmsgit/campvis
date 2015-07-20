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

#ifndef BUFFER_H__
#define BUFFER_H__

#include "cgt/exception.h"
#include "cgt/cgt_gl.h"
#include "cgt/types.h"

#include <set>

namespace cgt {
    class VertexAttribute;

    class CGT_API BufferObject {
    public:
        enum TargetType {
            ARRAY_BUFFER = GL_ARRAY_BUFFER,
            ELEMENT_ARRAY_BUFFER = GL_ELEMENT_ARRAY_BUFFER,
            TEXTURE_BUFFER = GL_TEXTURE_BUFFER,
            UNIFORM_BUFFER = GL_UNIFORM_BUFFER,
            SHADER_STORAGE_BUFFER = GL_SHADER_STORAGE_BUFFER
        };

        enum UsageType {
            USAGE_STATIC_DRAW = GL_STATIC_DRAW,
            USAGE_STATIC_READ = GL_STATIC_READ,
            USAGE_STATIC_COPY = GL_STATIC_COPY,

            USAGE_DYNAMIC_DRAW = GL_DYNAMIC_DRAW,
            USAGE_DYNAMIC_READ = GL_DYNAMIC_READ,
            USAGE_DYNAMIC_COPY = GL_DYNAMIC_COPY,

            USAGE_STREAM_DRAW = GL_STREAM_DRAW,
            USAGE_STREAM_READ = GL_STREAM_READ,
            USAGE_STREAM_COPY = GL_STREAM_COPY
        };

        enum BaseType {
            BYTE = GL_BYTE,
            UNSIGNED_BYTE = GL_UNSIGNED_BYTE,
            SHORT = GL_SHORT,
            UNSIGNED_SHORT = GL_UNSIGNED_SHORT,
            INT = GL_INT,
            UNSIGNED_INT = GL_UNSIGNED_INT,
            FLOAT = GL_FLOAT,
            DOUBLE = GL_DOUBLE
        };

        /**
         * Creates a new OpenGL buffer object and generates an ID for it.
         * \param   target      Target type of buffer object.
         * \param   usage       Usage of data.
         * \throw   cgt::Exception when OpenGL buffer generation failed.
         */
        BufferObject(TargetType target, UsageType usage) throw (cgt::Exception);

        /**
         * Disposes and deletes the OpenGL buffer object.
         */
        virtual ~BufferObject();


        /**
         * Gets the base data type in the buffer.
         * \return  _baseType
         */
        BaseType getBaseType() const;

        /**
         * Gets the target type of this buffer
         * \return  _targetType
         */
        TargetType getTargetType() const;

        /**
         * Gets the number of elements per element (1, 2, 3 or 4).
         * \return  _elementSize
         */
        size_t getElementSize() const;

        /**
         * Gets the buffer size in bytes on the GPU.
         * \return  _size
         */
        size_t getBufferSize() const;

        /**
         * Gets the OpenGL id of this buffer object;
         * \return _id
         */
        GLuint getId() const { return _id; };

        /**
         * Binds the buffer object to the current OpenGL context.
         * \param target    Target type of buffer object
         */
        void bind();

        /**
         * Binds the buffer, reserves space on the OpenGL device and initializes it with the given data in \a data.
         * \param data          Pointer to host memory containing the data.
         * \param numBytes      Size of \a data in bytes.
         * \param baseType      Base data type
         * \param elementSize   Number of elements per element (must be 1, 2, 3 or 4)
         */
        void data(const void* data, size_t numBytes, BaseType baseType, size_t elementSize);

        /**
         * Updates a subset of the buffer's data.
         * \param offset    Offset where to start writing in buffer's data in bytes.
         * \param data      Pointer to host memory containing the data.
         * \param numBytes  Size of \a data in bytes.
         */
        void subdata(size_t offset, const void* data, size_t numBytes);


    protected:
        GLuint _id;                         ///< OpenGL ID of this buffer

        TargetType _targetType;             ///< Target type of buffer object
        UsageType _usageType;               ///< Usage type of data.
        BaseType _baseType;                 ///< Base data type
        size_t _elementSize;                ///< Number of elements per element *lol* (must be 1, 2, 3 or 4)

        size_t _size;                       ///< Buffer size in bytes
        size_t _numElements;                ///< Number of elements in this buffer
    };
}

#endif // BUFFER_H__

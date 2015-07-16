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

#ifndef VERTEXATTRIBUTE_H__
#define VERTEXATTRIBUTE_H__

#include "cgt/exception.h"
#include "cgt/cgt_gl.h"
#include "cgt/types.h"

#include <string>
#include <vector>
#include <map>

namespace cgt {
    class BufferObject;
    class Shader;
    class VertexArrayObject;

    /**
     * Wrapper class for OpenGL Vertex Array Objects.
     * 
     * \see     http://www.opengl.org/wiki/Vertex_Array_Object
     * \note    Because all member methods require a valid locked OpenGL context, we have 
     *          implicit thread-safety.
     */
    class CGT_API VertexArrayObject {
    public:
        explicit VertexArrayObject(bool autoBind = true) throw (cgt::Exception);

        /**
         * Destructor, deletes the whole VAO.
         */
        ~VertexArrayObject();

        /**
         * Returns the OpneGL object ID
         * \return  _id
         */
        GLuint getId() const;

        /**
         * Binds this VertexArrayObject to the current OpenGL context.
         */
        void bind();

        /**
         * Unbinds this VertexArrayObject from the current OpenGL context.
         */
        void unbind();

        /**
         * Binds the provided buffer \a bufferObject as index buffer to this VAO.
         * \param   bufferObject    Buffer to use as element index buffer.
         */
        void bindIndexBuffer(BufferObject* bufferObject);

        /**
         * Add a VertexAttribute for \a bufferObject to this VertexArrayObject.
         * A VertexAttribute refers to a BufferObject and defines where to find the (geometry, normal, ...) data
         * in it.
         * \param   location        Location where this VertexAttribute is bound to the vertex shader.
         * \param   bufferObject    BufferObject to bind
         * \param   stride          Stride
         * \param   offset          Offset
         */
        void setVertexAttributePointer(GLuint location, BufferObject* bufferObject, GLsizei stride = 0, size_t offset = 0, bool enable = true);

        /**
         * Enables the VertexAttribute with the given location in the OpenGL client state.
         * \param   location   Index of the VertexAttribute to enable.
         */
        void enableVertexAttribute(GLuint location);

        /**
         * Enables the VertexAttribute assigned to the given BufferObject.
         * \note    The BufferObject has to be added to this VAO first using setVertexAttributePointer().
         * \param   bufferObject    BufferObject whose VertexAttribute shall be enabled
         */
        void enableVertexAttribute(BufferObject* bufferObject);

        /**
         * Disables the VertexAttribute with the given index in the OpenGL client state.
         * \param   location   Index of the VertexAttribute to disable.
         */
        void disableVertexAttribute(GLuint location);

        /**
         * Disables the VertexAttribute assigned to the given BufferObject.
         * \note    The BufferObject has to be added to this VAO first using setVertexAttributePointer().
         * \param   bufferObject    BufferObject whose VertexAttribute shall be disabled
         */
        void disableVertexAttribute(BufferObject* bufferObject);

    protected:

        GLuint _id;         ///< Id of the internal OpenGL handle to the VAO.

        std::map<BufferObject*, GLuint> _locationMap;   ///< Mapping added BufferObjects to locations.

        static const std::string loggerCat_;
    };
}

#endif // VERTEXATTRIBUTE_H__

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

#include "vertexarrayobject.h"

#include "cgt/assert.h"
#include "cgt/buffer.h"
#include "cgt/shadermanager.h"

namespace cgt {

    const std::string VertexArrayObject::loggerCat_ = "cgt.VertexArrayObject";

    VertexArrayObject::VertexArrayObject(bool autoBind) throw (cgt::Exception)
        : _id(0)
    {
        glGenVertexArrays(1, &_id);
        if (_id == 0) {
            throw new cgt::Exception("Could not create OpenGL Vertex Array.");
        }

        if (autoBind)
            bind();
    }

    VertexArrayObject::~VertexArrayObject() {
        glDeleteVertexArrays(1, &_id);
    }

    void VertexArrayObject::bind() {
        glBindVertexArray(_id);
    }

    void VertexArrayObject::unbind() {
        glBindVertexArray(0);
    }

    void VertexArrayObject::bindIndexBuffer(BufferObject* bufferObject) {
        cgtAssert(bufferObject->getTargetType() == BufferObject::ELEMENT_ARRAY_BUFFER, "Buffer needs to have target type ELEMENT_ARRAY_BUFFER!");

        bind();
        bufferObject->bind();
    }

    void VertexArrayObject::setVertexAttributePointer(GLuint location, BufferObject* bufferObject, GLsizei stride /*= 0*/, size_t offset /*= 0*/, bool enableNow /*= true*/) {
        cgtAssert(bufferObject != 0, "Pointer to buffer object must not be 0.");
        cgtAssert(bufferObject->getTargetType() == BufferObject::ARRAY_BUFFER, "Buffer must be bound to the ARRAY_BUFFER target!");

        // Todo: implement normalized flag if needed
        bind();
        bufferObject->bind();
        glVertexAttribPointer(location, static_cast<GLint>(bufferObject->getElementSize()), bufferObject->getBaseType(), false, stride, reinterpret_cast<void*>(offset));
        _locationMap.insert(std::make_pair(bufferObject, location));

        // enable if wanted
        if (enableNow)
            enableVertexAttribute(location);
    }

    void VertexArrayObject::enableVertexAttribute(GLuint location) {
        bind();
        glEnableVertexAttribArray(location);
    }

    void VertexArrayObject::enableVertexAttribute(BufferObject* bufferObject) {
        std::map<BufferObject*, GLuint>::const_iterator it = _locationMap.find(bufferObject);
        if (it != _locationMap.end())
            enableVertexAttribute(it->second);
        else
            cgtAssert(false, "Could not find Vertex Attribute location for this BufferObject. You have to add it first using setVertexAttributePointer()!");
    }

    void VertexArrayObject::disableVertexAttribute(GLuint location) {
        bind();
        glDisableVertexAttribArray(location);
    }

    void VertexArrayObject::disableVertexAttribute(BufferObject* bufferObject) {
        std::map<BufferObject*, GLuint>::const_iterator it = _locationMap.find(bufferObject);
        if (it != _locationMap.end())
            disableVertexAttribute(it->second);
        else
            cgtAssert(false, "Could not find Vertex Attribute location for this BufferObject. You have to add it first using setVertexAttributePointer()!");
    }

    GLuint VertexArrayObject::getId() const {
        return _id;
    }

}
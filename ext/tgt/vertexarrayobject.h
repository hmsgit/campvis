#ifndef VERTEXATTRIBUTE_H__
#define VERTEXATTRIBUTE_H__

#include "tgt/exception.h"
#include "tgt/tgt_gl.h"

#include <string>
#include <vector>
#include <map>

namespace tgt {
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
    class VertexArrayObject {
    public:
        VertexArrayObject(bool autoBind = true) throw (tgt::Exception);

        /**
         * Destructor, deletes the whole VAO.
         */
        ~VertexArrayObject();

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

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
     * Class encapsulating an OpenGL Vertex Attribute state.
     * \see http://www.opengl.org/wiki/Vertex_Array_Object
     */
    class VertexAttribute {
        friend class VertexArrayObject;

    public:
        /**
         * Destructor
         */
        ~VertexAttribute();

    protected:
        /**
         * Creates an OpenGL Vertex Attribute for the data in \a BufferObject and binds it to the uniform
         * named \a uniformName in the given shader.
         * 
         * \note    VertexAttributes shall only be created by VertexArrayObject's member functions, hence the
         *          constructor is protected.
         *          
         * \param   index         Index of the generic vertex attribute to be modified.
         * \param   bufferObject  BufferObject to bind
         */
        VertexAttribute(GLuint index, BufferObject* bufferObject, GLsizei stride, size_t offset);

        GLuint _index;                      ///< Index of the generic vertex attribute to be modified.
        GLsizei _stride;                    ///< Specifies the byte offset between consecutive generic vertex attributes.
        size_t _offset;                     ///< Offset in bytes to the first element in the buffer.

        BufferObject* _bufferObject;        ///< BufferObject that is bound to this VertexAttribute.

        static const std::string loggerCat_;
    };

// ================================================================================================

    /**
     * Wrapper class for OpenGL Vertex Array Objects.
     * 
     * \note    Because all member methods require a valid locked OpenGL context, we have 
     *          implicit thread-safety.
     */
    class VertexArrayObject {
    public:
        /**
         * Enumeration of different VertexAttribute types.
         * Every added VertexAttribute may have the UnspecifiedAttribute, then its index cannot be
         * re-acquired by getVertexAttributeIndexByType(). In contrast, each other attribute types
         * can only be assigned to one single VertexAttribute which then can be re-acquired by 
         * getVertexAttributeIndexByType().
         */
        enum AttributeType {
            UnspecifiedAttribute,
            VerticesAttribute,
            NormalsAttribute,
            TextureCoordinatesAttribute,
            ColorsAttribute
        };

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

        void setIndexBuffer(BufferObject* bufferObject);

        /**
         * Add a VertexAttribute to this VertexArrayObject.
         * A VertexAttribute refers to a BufferObject and defines where to find the (geometry, normal, ...) data
         * in it.
         * \param   attributeType 
         * \param   bufferObject 
         * \param   stride 
         * \param   offset 
         * \return  
         */
        size_t addVertexAttribute(AttributeType attributeType, BufferObject* bufferObject, GLsizei stride = 0, size_t offset = 0, bool enable = true);

        /**
         *
         * \param index 
         * \param bufferObject 
         * \param stride 
         * \param offset 
         */
        void updateVertexAttribute(size_t index, BufferObject* bufferObject, GLsizei stride = 0, size_t offset = 0);

        /**
         * Enables the VertexAttribute with the given index in the OpenGL client state.
         * \param   index   Index of the VertexAttribute to enable.
         */
        void enableVertexAttribute(size_t index);

        /**
         * Enables all VertexAttributes of this VAO.
         */
        void enableAllVertexAttributes();

        /**
         * Disables the VertexAttribute with the given index in the OpenGL client state.
         * \param   index   Index of the VertexAttribute to disable.
         */
        void disableVertexAttribute(size_t index);

        /**
         * Disables all VertexAttributes of this VAO.
         */
        void disableAllVertexAttributes();

        /**
         * Returns the index of the VertexAttribute which was previously created with the given type.
         * \note    A VertexAttribute of type \a type must have been created beforehand. Otherwise
         *          this method will raise an error and just return 0 (which is also a perfectly 
         *          valid index).
         * \param   type    Type of the VertexAttribute to search for. Must not be UnspecifiedAttribute, must have been created beforehand!
         * \return  The index of the VertexAttribute, which was created with the given type. If no such
         *          VA was created, this method raises an error and returns 0 as default value (which is
         *          also a perfect valid return value!).
         */
        size_t getVertexAttributeIndexByType(AttributeType type) const;

    protected:
        /**
         * Initializes all static members.
         */
        static void initStaticMembers();

        GLuint _id;                                         ///< Id of the internal OpenGL handle to the VAO.
        std::vector<VertexAttribute> _attributes;           ///< List of all VertexAttributes of this VAO.
        std::vector<bool> _enabledAttributes;               ///< List of the enabled VertexAttributes.
        std::map<AttributeType, size_t> _attributeTypeMap;  ///< Map of the AttributeTypes to their indices in the VertexAttributes list.

        static bool _initialized;                           ///< Flag whether the static members are initialized;
        static size_t _currentlyBoundVertexArray;           ///< Id of the VertexArrayObject which was bound most recently. 0 if the most recent bind call was unbind().

        static const std::string loggerCat_;
    };
}

#endif // VERTEXATTRIBUTE_H__

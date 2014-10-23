#include "vertexarrayobject.h"

#include "cgt/assert.h"
#include "cgt/buffer.h"
#include "cgt/shadermanager.h"

namespace tgt {

    const std::string VertexArrayObject::loggerCat_ = "tgt.VertexArrayObject";

    VertexArrayObject::VertexArrayObject(bool autoBind) throw (tgt::Exception)
        : _id(0)
    {
        glGenVertexArrays(1, &_id);
        if (_id == 0) {
            throw new tgt::Exception("Could not create OpenGL Vertex Array.");
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
        tgtAssert(bufferObject->getTargetType() == BufferObject::ELEMENT_ARRAY_BUFFER, "Buffer needs to have target type ELEMENT_ARRAY_BUFFER!");

        bind();
        bufferObject->bind();
    }

    void VertexArrayObject::setVertexAttributePointer(GLuint location, BufferObject* bufferObject, GLsizei stride /*= 0*/, size_t offset /*= 0*/, bool enableNow /*= true*/) {
        tgtAssert(bufferObject != 0, "Pointer to buffer object must not be 0.");
        tgtAssert(bufferObject->getTargetType() == BufferObject::ARRAY_BUFFER, "Buffer must be bound to the ARRAY_BUFFER target!");

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
            tgtAssert(false, "Could not find Vertex Attribute location for this BufferObject. You have to add it first using setVertexAttributePointer()!");
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
            tgtAssert(false, "Could not find Vertex Attribute location for this BufferObject. You have to add it first using setVertexAttributePointer()!");
    }

    GLuint VertexArrayObject::getId() const {
        return _id;
    }

}
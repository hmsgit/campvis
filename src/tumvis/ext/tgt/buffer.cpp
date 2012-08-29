#include "buffer.h"


namespace tgt {
    BufferObject::BufferObject()
        : _id(0)
        , _size(0)
        , _numElements(0)
    {
        glGenBuffers(1, &_id);
        if (_id == 0) {
            throw new tgt::Exception("Could not create OpenGL Buffer object.");
        }
    }

    BufferObject::~BufferObject() {
        tgtAssert(_assignedAttributes.empty(), "Destructing a BufferObject, that is still bound to a VertexAttribute.");

        if (_id != 0)
            glDeleteBuffers(1, &_id);
    }

    void BufferObject::bind(TargetType target) {
        glBindBuffer(target, _id);
    }

    void BufferObject::data(TargetType target, UsageType usage, const void* data, size_t numBytes) {
        bind(target);
        glBufferData(target, numBytes, data, usage);
    }

    void BufferObject::subdata(TargetType target, UsageType usage, size_t offset, const void* data, size_t numBytes) {
        bind(target);
        glBufferSubData(target, offset, numBytes, data);
    }

    void BufferObject::bindToVertexAttribute(const VertexAttribute* va) {
        _assignedAttributes.insert(va);
    }

    void BufferObject::unbindFromVertexAttribute(const VertexAttribute* va) {
        _assignedAttributes.erase(va);
    }

}
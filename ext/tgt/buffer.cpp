#include "buffer.h"
#include "tgt/openglgarbagecollector.h"


namespace tgt {
    BufferObject::BufferObject(TargetType target, UsageType usage) throw (tgt::Exception)
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
            throw new tgt::Exception("Could not create OpenGL Buffer object.");
        }
    }

    BufferObject::~BufferObject() {
        tgtAssert(_assignedAttributes.empty(), "Destructing a BufferObject, that is still bound to a VertexAttribute.");

        if (_id != 0) {
            GLGC.addGarbageBufferObject(_id);
        }
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

    void BufferObject::bindToVertexAttribute(const VertexAttribute* va) {
        _assignedAttributes.insert(va);
    }

    void BufferObject::unbindFromVertexAttribute(const VertexAttribute* va) {
        _assignedAttributes.erase(va);
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
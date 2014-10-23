#include "buffer.h"
#include "cgt/openglgarbagecollector.h"


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
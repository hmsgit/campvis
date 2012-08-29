#include "vertexattribute.h"

#include "tgt/assert.h"
#include "tgt/buffer.h"
#include "tgt/shadermanager.h"

namespace tgt {

    const std::string VertexAttribute::loggerCat_ = "tgt.VertexAttribute";

    VertexAttribute::VertexAttribute(size_t index, BufferObject* bufferObject, size_t stride, size_t offset)
        : _index(index)
        , _stride(stride)
        , _offset(offset)
        , _bufferObject(bufferObject)
    {
        tgtAssert(_bufferObject != 0, "BufferObject must not be 0.");

        _bufferObject->bindToVertexAttribute(this);
        // Todo: implement normalized flag if needed
        glVertexAttribPointer(_index, _bufferObject->getElementSize(), _bufferObject->getBaseType(), false, _stride, reinterpret_cast<void*>(_offset));
    }

    VertexAttribute::~VertexAttribute() {
        _bufferObject->unbindFromVertexAttribute(this);
    }

// ================================================================================================

    const std::string VertexArrayObject::loggerCat_ = "tgt.VertexArrayObject";

    VertexArrayObject::VertexArrayObject(bool autoBind) 
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
        if (!_initialized)
            initStaticMembers();

        if (_currentlyBoundVertexArray != _id) {
            glBindVertexArray(_id);
            _currentlyBoundVertexArray = _id;
        }
    }

    void VertexArrayObject::unbind() {
        if (!_initialized)
            initStaticMembers();

        if (_currentlyBoundVertexArray != 0) {
            glBindVertexArray(0);
            _currentlyBoundVertexArray = 0;
        }
    }

    size_t VertexArrayObject::addVertexAttribute(AttributeType attributeType, BufferObject* bufferObject, size_t stride /*= 0*/, size_t offset /*= 0*/, bool enableNow /*= true*/) {
        if (_attributes.size() > 16) {
            // TODO:    The better way would be to check glGet(GL_MAX_VERTEX_ATTRIBS), but the standard says 16 is the minimum
            //          number to be supported and that should be enough and I currently feel lazy. If you're reading this, 
            //          feel free to improve this check...
            LERROR("Could not add VertexAttribute: Tried to add more Vertex Attributes than supported.");
            return 0;
        }

        tgtAssert(
            attributeType != UnspecifiedAttribute && _attributeTypeMap.find(attributeType) != _attributeTypeMap.end(), 
            "Tried to add two VertexAttributes with the same type. This is currently not supported.");

        // bind and create VertexAttribute
        bind();
        size_t index = _attributes.size();
        _attributes.push_back(VertexAttribute(index, bufferObject, stride, offset));

        // add to attribute-type map
        if (attributeType != UnspecifiedAttribute)
            _attributeTypeMap.insert(std::make_pair(attributeType, index));

        // enable if wanted
        if (enableNow)
            enableVertexAttribute(index);

        return index;
    }

    void VertexArrayObject::updateVertexAttribute(size_t index, BufferObject* bufferObject, size_t stride /*= 0*/, size_t offset /*= 0*/) {
        tgtAssert(index < _attributes.size(), "Index out of bounds.");

        bind();
        _attributes[index] = VertexAttribute(index, bufferObject, stride, offset);
    }

    void VertexArrayObject::enableVertexAttribute(size_t index) {
        tgtAssert(index < _enabledAttributes.size(), "Index out of bounds.");

        if (!_initialized)
            initStaticMembers();

        glEnableVertexAttribArray(index);
        _enabledAttributes[index] = true;
    }

    void VertexArrayObject::disableVertexAttribute(size_t index) {
        tgtAssert(index < _enabledAttributes.size(), "Index out of bounds.");

        if (!_initialized)
            initStaticMembers();

        glDisableVertexAttribArray(index);
        _enabledAttributes[index] = false;
    }

    void VertexArrayObject::initStaticMembers() {
        // TODO:    The better way would be to us glGet(GL_MAX_VERTEX_ATTRIBS) as dimension, but the standard says 16 
        //          is the minimum number to be supported and that should be enough and I currently feel lazy. If 
        //          you're reading this, feel free to improve this allocation...
        _enabledAttributes = std::vector<bool>(16, false);
        _currentlyBoundVertexArray = 0;
        _initialized = true;
    }

    size_t VertexArrayObject::getVertexAttributeIndexByType(AttributeType type) const {
        tgtAssert(type != UnspecifiedAttribute, "Type most not be UnspecifiedAttribute, those attributes are not tracked.");

        std::map<AttributeType, size_t>::const_iterator it = _attributeTypeMap.find(type);
        if (it == _attributeTypeMap.end()) {
            tgtAssert(false, "Could not find a VertexAttribute with the given type. Returning 0 as default value.");
            LERROR("Could not find a VertexAttribute with the given type. Returning 0 as default value.");
            return 0;
        }
        return it->second;
    }


}
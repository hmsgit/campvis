// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
// 
// ================================================================================================

#ifndef GEOMETRYDATA_H__
#define GEOMETRYDATA_H__

#include "cgt/cgt_gl.h"
#include "cgt/bounds.h"
#include "core/datastructures/abstractdata.h"
#include <vector>

namespace cgt {
    class BufferObject;
    class GLCanvas;
}

namespace campvis {

    /**
     * Abstract base class for geometry data in CAMPVis.
     * 
     * GeometryData consists of a set of vertices (vec3) and optionally colors (vec4), texture 
     * coordinates (vec3) and/or normals (vec3) for each vertex.
     * GeometryData stores the geometry on the CPU side (local RAM) and takes care of transparently 
     * mapping it into GPU memory in order to render it using OpenGL. The mapping of vertex 
     * information to OpenGL vertex attributes is as follows:
     *  - Vertex positions: Vertex attribute 0
     *  - Vertex texture coordinates: Vertex attribute 1
     *  - Vertex colors: Vertex attribute 2
     *  - Vertex normals: Vertex attribute 3
     * 
     */    
    class CAMPVIS_CORE_API GeometryData : public AbstractData, public IHasWorldBounds {
    public:
        /**
         * Constructor
         * \param   vertexFeatures  List of features present for each vertex
         */
        explicit GeometryData();

        /**
         * Copy constructor
         * \param   rhs GeometryData to copy
         */
        GeometryData(const GeometryData& rhs);

        /**
         * Destructor, deletes VBOs/VAO if necessary.
         */
        virtual ~GeometryData();

        /**
         * Assignment operator.
         * \param   rhs GeometryData to assign to this.
         * \return  *this after assignment
         */
        GeometryData& operator=(const GeometryData& rhs);

        /// \see AbstractData::clone()
        virtual GeometryData* clone() const = 0;

        /**
         * Renders this GeometryData.
         * Must be called from a valid OpenGL context.
         */
        virtual void render(GLenum mode = GL_TRIANGLE_FAN) const = 0;
        
        /**
         * Returns the geometry extent in world coordinates.
         * \return  The geometry extent in world coordinates.
         */
        virtual cgt::Bounds getWorldBounds() const = 0;


        /**
         * Returns whether the geometry has texture coordinates.
         * \return  True if this geometry sets texture coordinates during rendering.
         */
        virtual bool hasTextureCoordinates() const = 0;

        /**
         * Returns whether this geometry has picking information.
         * \return  True if this geometry sets picking information during rendering.
         */
        virtual bool hasPickingInformation() const = 0;

        /**
         * Applies the transformation matrix \a t to each vertex of this geometry.
         * \param   t   Transformation matrix to apply
         */
        virtual void applyTransformationToVertices(const cgt::mat4& t) = 0;

        /**
         * Returns the Pointer to the OpenGL Buffer with the vertex positions.
         * May be 0 if not yet created.
         * \return  _verticesBuffer
         */
        const cgt::BufferObject* getVerticesBuffer() const;

        /**
         * Returns the Pointer to the OpenGL Buffer with the vertex texture coordinates.
         * May be 0 if none are present or not yet created.
         * \return  _texCoordsBuffer
         */
        const cgt::BufferObject* getTextureCoordinatesBuffer() const;

        /**
         * Returns the Pointer to the OpenGL Buffer with the vertex colors. 
         * May be 0 if none are present or not yet created.
         * \return  _colorsBuffer
         */
        const cgt::BufferObject* getColorsBuffer() const;

        /**
         * Returns the Pointer to the OpenGL Buffer with the vertex normals.
         * May be 0 if none are present or not yet created.
         * \return  _normalsBuffer
         */
        const cgt::BufferObject* getNormalsBuffer() const;

        /**
         * Returns the Pointer to the OpenGL Buffer with the vertex normals.
         * May be 0 if none are present or not yet created.
         * \return  _normalsBuffer
         */
        const cgt::BufferObject* getPickingBuffer() const;

        /// \see AbstractData::getVideoMemoryFootprint()
        virtual size_t getVideoMemoryFootprint() const;

    protected:
        /**
         * Deletes all OpenGL BufferObjects.
         */
        void deleteBuffers() const;


        // mutable to support const lazy initialization
        mutable bool _buffersDirty;             ///< Flag whether the buffers are dirty (i.e. need to be (re)initialized)

        enum { NUM_BUFFERS = 5 };               ///< Number of buffers in _buffers array

        union {
            struct {
                mutable cgt::BufferObject* _verticesBuffer;     ///< Pointer to the OpenGL Buffer with the vertex positions
                mutable cgt::BufferObject* _texCoordsBuffer;    ///< Pointer to the OpenGL Buffer with the vertex texture coordinates
                mutable cgt::BufferObject* _colorsBuffer;       ///< Pointer to the OpenGL Buffer with the vertex colors
                mutable cgt::BufferObject* _normalsBuffer;      ///< Pointer to the OpenGL Buffer with the vertex normals
                mutable cgt::BufferObject* _pickingBuffer;      ///< Pointer to the OpenGL Buffer with the picking information
            };

            mutable cgt::BufferObject* _buffers[NUM_BUFFERS];   ///< Array of all buffers
        };

    private:

        static const std::string loggerCat_;
    };

}

/*
=== This is a draft for a new, generic GeometryData class design that manages arbitrary OpenGL buffers ====

class CAMPVIS_CORE_API GeometryDataBase {
public:
    /// Enumeration for defining semantics of stored buffer data
    enum ElementSemantic {
        VERTEX                  = 0,    ///< Vextex data
        TEXTURE_COORDINATE      = 1,    ///< Texture coordinate data
        COLOR                   = 2,    ///< Color data
        NORMAL                  = 3,    ///< Normal data
        PICKING_INFORMATION     = 4     ///< Picking information
    };

    /// Enumeration for defining the host data type of the element
    enum ElementHostType {
        UINT8,
        UINT16,
        UINT32,
        FLOAT,
        VEC2,
        VEC3,
        VEC4
    };
};

template<GeometryDataBase::ElementSemantic SEMANTIC>
struct GeometryDataTraits {};

template<>
struct GeometryDataTraits<GeometryDataBase::VERTEX> {
    typedef cgt::vec3 HostType;
};

class CAMPVIS_CORE_API DraftNewGeometryData {
public:
    template<GeometryDataBase::ElementSemantic SEMANTIC>
    const std::vector<typename GeometryDataTraits<SEMANTIC>::HostType>* getElementData() const;

    template<GeometryDataBase::ElementSemantic SEMANTIC>
    void setElementData(std::vector<typename GeometryDataTraits<SEMANTIC>::HostType>* elementData);

protected:
    std::vector<void*> _elementPointers;
};


template<GeometryDataBase::ElementSemantic SEMANTIC>
const std::vector<typename GeometryDataTraits<SEMANTIC>::HostType>* GeometryData::getElementData() const {
    if (_elementPointers.size() >= SEMANTIC) {
        return static_cast< std::vector<typename GeometryDataTraits<SEMANTIC>::HostType>* >(_elementPointers[SEMANTIC]);
    }

    return nullptr;
}

template<GeometryDataBase::ElementSemantic SEMANTIC>
void GeometryData::setElementData(std::vector<typename GeometryDataTraits<SEMANTIC>::HostType>* elementData) {
    if (_elementPointers.size() < SEMANTIC + 1)
        _elementPointers.resize(SEMANTIC, nullptr);

    void* oldPtr = _elementPointers[SEMANTIC];
    if (oldPtr != elementData)
        delete static_cast< std::vector<typename GeometryDataTraits<SEMANTIC>::HostType>* >(oldPtr);

    _elementPointers[SEMANTIC] = elementData;
}

*/


#endif // GEOMETRYDATA_H__

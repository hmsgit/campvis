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

#ifndef GEOMETRYDATACOLLECTION_H__
#define GEOMETRYDATACOLLECTION_H__

#include "core/datastructures/geometrydata.h"
#include <vector>

namespace cgt {
    class BufferObject;
    class GLCanvas;
}

namespace campvis {
    /**
     * GeometryData that holds a collection of independent GeometryData objects.
     * All geometries of this collection are rendered (and handled) at once.
     */    
    class CAMPVIS_CORE_API GeometryDataCollection : public GeometryData {
    public:
        /**
         * Constructor
         * \param   vertexFeatures  List of features present for each vertex
         */
        explicit GeometryDataCollection();

        /**
         * Copy constructor
         * \param   rhs GeometryDataCollection to copy
         */
        GeometryDataCollection(const GeometryDataCollection& rhs);

        /**
         * Destructor, deletes VBOs/VAO if necessary.
         */
        virtual ~GeometryDataCollection();

        /**
         * Assignment operator.
         * \param   rhs GeometryDataCollection to assign to this.
         * \return  *this after assignment
         */
        GeometryDataCollection& operator=(const GeometryDataCollection& rhs);

        /// \see AbstractData::clone()
        virtual GeometryDataCollection* clone() const;

        /**
         * Adds the GeometryData \a g to this collection.
         * \param   g   Geometry to add.
         */
        void addGeometry(GeometryData* g);

        /**
         * Returns the number of held geometries.
         * \return  _geometries.size()
         */
        size_t getNumGeometries() const;
        /**
         * Returns the geometry with index \a index of this collection.
         * \param   index   Index of the geometry to return.
         * \return  _geometries[index]
         */
        const GeometryData* getGeometry(size_t index) const;

        /**
         * Removes geometry with index \a index from this collection.
         * \param   index   Index of geometry to remove.
         */
        void removeGeometry(size_t index);

        /**
         * Renders this GeometryDataCollection.
         * Must be called from a valid OpenGL context.
         */
        virtual void render(GLenum mode = GL_TRIANGLE_FAN) const;
        
        /**
         * Returns the geometry extent in world coordinates.
         * \return  The geometry extent in world coordinates.
         */
        virtual cgt::Bounds getWorldBounds() const;

        /**
         * Returns whether all geometries of the collection have texture coordinates.
         * \return  True if all geometries of the collection set texture coordinates during rendering.
         */
        virtual bool hasTextureCoordinates() const;

        /**
         * Returns whether all geometries of the collection have picking information.
         * \return  True if all geometries of the collection set picking information during rendering.
         */
        virtual bool hasPickingInformation() const;

        /**
         * Applies the transformation matrix \a t to each vertex of this geometry.
         * \param   t   Transformation matrix to apply
         */
        virtual void applyTransformationToVertices(const cgt::mat4& t);

        /// \see AbstractData::getLocalMemoryFootprint()
        virtual size_t getLocalMemoryFootprint() const;
        /// \see AbstractData::getVideoMemoryFootprint()
        virtual size_t getVideoMemoryFootprint() const;
        /// \see AbstractData::getTypeAsString()
        virtual std::string getTypeAsString() const;

    protected:
        /// Collection of the geometries this object has.
        std::vector<GeometryData*> _geometries;

    private:

        static const std::string loggerCat_;
    };

}

#endif // GEOMETRYDATACOLLECTION_H__

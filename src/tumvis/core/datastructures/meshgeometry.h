// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FLOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#ifndef MESHGEOMETRY_H__
#define MESHGEOMETRY_H__

#include "tgt/bounds.h"
#include "tgt/vector.h"
#include "core/datastructures/geometrydata.h"
#include "core/datastructures/facegeometry.h"

#include <vector>

namespace TUMVis {

    /**
     * Class for mesh geometry.
     * Every MeshGeometry consists of a list of FaceGeometry objects.
     * 
     * The internal OpenGL buffers are lazy-instantiated.
     * 
     * \note    Like all Geometry classes MeshGeometry has value-sematics: Once created, the 
     *          faces cannot be altered anymore.
     */
    class MeshGeometry : public GeometryData {
    public:
        /**
         * Creates a new MeshGeometry built from the given faces.
         */
        MeshGeometry(const std::vector<FaceGeometry>& faces);

        /**
         * Destructor, deletes VBOs/VAO if necessary. Hence, needs a valid OpenGL context
         */
        virtual ~MeshGeometry();

        /// \see AbstractData::clone()
        virtual MeshGeometry* clone() const;

        /**
         * Returns the number of faces of this mesh.
         * \return _faces.size()
         */
        size_t size() const;

        /**
         * Gets the list of all faces of this mesh.
         * \return  _faces
         */
        const std::vector<FaceGeometry>& getFaces() const;

        /**
         * Clips this MeshGeometry against an aribtrary clip plane.
         * The clipped MeshGeometry is returned.
         * \param   p       Point on clip plane
         * \param   normal  Clip plane normal
         * \param   epsilon Clipping precision
         * \return  The clipped MeshGeometry
         */
        MeshGeometry clipAgainstPlane(float p, const tgt::vec3& normal, bool close = true, float epsilon = 1e-8f) const;

        /**
         * Renders this MeshGeometry.
         * Must be called from a valid OpenGL context.
         */
        virtual void render() const;

        /**
         * Creates the OpenGL VBOs and the VAO for this face's geometry.
         * Must be called from a valid OpenGL context.
         */
        virtual void createGLBuffers() const;

        /**
         * Creates a MeshGeometry building a cube with the given bounds and texture coordinates.
         * \note    The caller owns the returned pointer.
         * \param   bounds      coordinates of the cube bounds
         * \param   texBounds   texture coordinates at the cube bounds
         * \return  A MeshGeometry building a cube with the given bounds and texture coordinates
         */
        static MeshGeometry createCube(const tgt::Bounds& bounds, const tgt::Bounds& texBounds);

    protected:

        std::vector<FaceGeometry> _faces;           ///< List of all faces of this mesh

        static const std::string loggerCat_;
    };

}

#endif // MESHGEOMETRY_H__
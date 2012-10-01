// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#ifndef GEOMETRYTRANSFERFUNCTION_H__
#define GEOMETRYTRANSFERFUNCTION_H__

#include "core/classification/abstracttransferfunction.h"

#include <vector>

namespace TUMVis {

    class TFGeometry;

    /**
     * A 1D transfer function built from multiple geometries.
     */
    class GeometryTransferFunction : public AbstractTransferFunction, public sigslot::has_slots<> {
    public:
        /**
         * Creates a new GeometryTransferFunction.
         * \param   size            Size of the transfer function texture
         * \param   intensityDomain Intensity Domain where the transfer function is mapped to during classification
         */
        GeometryTransferFunction(size_t size, const tgt::vec2& intensityDomain = tgt::vec2(0.f, 1.f));

        /**
         * Destructor, make sure to delete the OpenGL texture beforehand by calling deinit() with a valid OpenGL context!
         */
        virtual ~GeometryTransferFunction();

        /**
         * Returns the dimensionality of the transfer function.
         * \return  The dimensionality of the transfer function.
         */
        virtual size_t getDimensionality() const;

        /**
         * Gets the list of transfer function geometries.
         * \return  _geometries
         */
        const std::vector<TFGeometry*>& getGeometries() const;

        /**
         * Adds the given TF geometry to this transfer function.
         * \note    GeometryTransferFunction takes ownership \a geometry.
         * \param   geometry    TF geometry to add, GeometryTransferFunction takes the ownership.
         */
        void addGeometry(TFGeometry* geometry);

        /**
         * Removes the given TF geometry from this transfer function.
         * \note    After the call \a geometry will no longer be valid as GeometryTransferFunction deletes the given TFGeometry.
         * \param   geometry    TF geometry to remove, GeometryTransferFunction will delete it
         */
        void removeGeometry(TFGeometry* geometry);

        /**
         * Slot to be called by TFGeometry's s_changed signal.
         */
        void onGeometryChanged();

        /// Signal to be emitted when the vector of TFGeometry objects (_geometries) changed (The collection, not the actual geometry).
        sigslot::signal0<> s_geometryCollectionChanged;

    protected:
        /**
         * Creates the texture and uploads it to OpenGL.
         * Gets called by bind() with the local mutex already acquired.
         */
        virtual void createTexture();

        std::vector<TFGeometry*> _geometries;       ///< The list of transfer function geometries.

        static const std::string loggerCat_;

    };

}

#endif // GEOMETRYTRANSFERFUNCTION_H__

// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#ifndef GEOMETRY2DTRANSFERFUNCTION_H__
#define GEOMETRY2DTRANSFERFUNCTION_H__

#include "core/classification/genericgeometrytransferfunction.h"

#include <vector>

namespace campvis {

    class TFGeometry2D;

    /**
     * A 2D transfer function built from multiple geometries.
     */
    class CAMPVIS_CORE_API Geometry2DTransferFunction : public GenericGeometryTransferFunction<TFGeometry2D> {
    public:
        /**
         * Creates a new Geometry2DTransferFunction.
         * \param   size            Size of the transfer function texture
         * \param   intensityDomain Intensity Domain where the transfer function is mapped to during classification
         */
        explicit Geometry2DTransferFunction(const cgt::svec2& size, const cgt::vec2& intensityDomain = cgt::vec2(0.f, 1.f));

        /**
         * Destructor, make sure to delete the OpenGL texture beforehand by calling deinit() with a valid OpenGL context!
         */
        virtual ~Geometry2DTransferFunction();
                
        /**
         * Clones this transfer function.
         * \return  A clone of this transfer function, caller takes ownership of the pointer.
         */
        virtual Geometry2DTransferFunction* clone() const;

        /**
         * Returns the dimensionality of the transfer function.
         * \return  The dimensionality of the transfer function.
         */
        virtual size_t getDimensionality() const;


    protected:

        static const std::string loggerCat_;

    };

}

#endif // GEOMETRY2DTRANSFERFUNCTION_H__

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

#ifndef SIMPLETRANSFERFUNCTION_H__
#define SIMPLETRANSFERFUNCTION_H__

#include "core/classification/abstracttransferfunction.h"

namespace campvis {

    /**
     * A very simple ramp transfer function, just for testing purposes...
     */
    class CAMPVIS_CORE_API SimpleTransferFunction : public AbstractTransferFunction {
    public:
        /**
         * Creates a new SimpleTransferFunction.
         * \param   size            Size of the transfer function texture
         * \param   intensityDomain Intensity Domain where the transfer function is mapped to during classification
         */
        explicit SimpleTransferFunction(size_t size, const cgt::vec2& intensityDomain = cgt::vec2(0.f, 1.f));

        /**
         * Destructor, make sure to delete the OpenGL texture beforehand by calling deinit() with a valid OpenGL context!
         */
        virtual ~SimpleTransferFunction();
        
        /**
         * Clones this transfer function.
         * \return  A clone of this transfer function, caller takes ownership of the pointer.
         */
        virtual SimpleTransferFunction* clone() const;

        /**
         * Returns the dimensionality of the transfer function.
         * \return  The dimensionality of the transfer function.
         */
        virtual size_t getDimensionality() const;

        const cgt::col4& getLeftColor() const;
        void setLeftColor(const cgt::col4& color);
        const cgt::col4& getRightColor() const;
        void setRightColor(const cgt::col4& color);

    protected:
        /**
         * Creates the texture and uploads it to OpenGL.
         * Gets called by bind() with the local mutex already acquired.
         */
        virtual void createTexture();

        cgt::col4 _leftColor;
        cgt::col4 _rightColor;

        static const std::string loggerCat_;

    };

}

#endif // SIMPLETRANSFERFUNCTION_H__

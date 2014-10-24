// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
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

#ifndef CAMERAPROVIDER_H__
#define CAMERAPROVIDER_H__

#include <string>

#include "tgt/camera.h"
#include "core/pipeline/abstractprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/optionproperty.h"

namespace campvis {
    /**
     * Generates CameraData objects.
     */
    class CameraProvider : public AbstractProcessor {
    public:
        /**
         * Constructs a new CameraProvider Processor
         **/
        CameraProvider();

        /**
         * Destructor
         **/
        virtual ~CameraProvider();


        /// \see AbstractProcessor::updateResult()
        virtual void updateResult(DataContainer& dataContainer);

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "CameraProvider"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Generates CameraData objects."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        DataNameProperty p_cameraId;        ///< Name/ID for the CameraData object

        Vec3Property p_position;            /// location of the camera
        Vec3Property p_focus;               /// location, the camera looks at
        Vec3Property p_upVector;            /// up vector, always normalized

        FloatProperty p_fov;                ///< Field of View
        FloatProperty p_aspectRatio;        ///< Aspect Ratio
        Vec2Property p_clippingPlanes;      ///< Near and far clipping planes

        GenericOptionProperty<tgt::Camera::ProjectionMode> p_projectionMode;   ///< Projection mode

    protected:

        static const std::string loggerCat_;
    };

}

#endif // CAMERAPROVIDER_H__
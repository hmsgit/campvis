// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#ifndef TRACKEDUSSWEEPFRAMERENDERER3D_H__
#define TRACKEDUSSWEEPFRAMERENDERER3D_H__

#include <string>

#include "tgt/bounds.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/buttonproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/transferfunctionproperty.h"
#include "core/properties/cameraproperty.h"

class TrackedUSFileIO;
class TrackedUSSweep;

namespace tgt {
    class Shader;
}

namespace campvis {
    class ImageData;

    /**
     * Extracts a slice from a 3D image and renders it into a rendertarget.
     */
    class TrackedUsSweepFrameRenderer3D : public VisualizationProcessor {
    public:
        /**
         * Constructs a new TrackedUsSweepFrameRenderer3D Processor
         **/
        TrackedUsSweepFrameRenderer3D(IVec2Property& canvasSize);

        /**
         * Destructor
         **/
        virtual ~TrackedUsSweepFrameRenderer3D();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "TrackedUsSweepFrameRenderer3D"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Extracts a single slice from the input image and renders it using a transfer function."; };

        /// \see AbstractProcessor::process()
        virtual void process(DataContainer& data);

        DataNameProperty p_sourceImageID;                  ///< image ID for input FileIO
        DataNameProperty p_targetImageID;                  ///< image ID for output image

        CameraProperty p_camera;
        IntProperty p_sweepNumber;                          ///< Number of the sweep to display
        IntProperty p_frameNumber;                          ///< Number of the frame to display
        BoolProperty p_showConfidenceMap;                   ///< Flag whether to show confidence map instead of US image
        ButtonProperty p_smoothButton;

        TransferFunctionProperty p_transferFunction;     ///< Transfer function

        /// adapts the range of the p_frameNumber property to the image
        void updateProperties(TrackedUSFileIO* fio);

        const TrackedUSSweep* getCurrentSweep() const;

        sigslot::signal1<tgt::Bounds> s_boundingBoxChanged;

    protected:
        void onSmoothButtonClicked();

        void updateBoundingBox();

        tgt::Shader* _shader;                           ///< Shader for slice rendering
        TrackedUSSweep* _currentSweep;
        tgt::Bounds _bounds;

        static const std::string loggerCat_;
    };
}

#endif // TRACKEDUSSWEEPFRAMERENDERER3D_H__

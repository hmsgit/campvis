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

#ifndef MANUALTISSUESEGMENTER_H__
#define MANUALTISSUESEGMENTER_H__

#include <string>
#include <map>

#include "tgt/bounds.h"
#include "tgt/event/eventlistener.h"

#include "core/datastructures/datahandle.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/buttonproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/transferfunctionproperty.h"
#include "core/properties/cameraproperty.h"
#include "core/properties/stringproperty.h"

#include "modules/manualsegmentation/tools/tissuesegmentation.h"

namespace tgt {
    class Shader;
}

namespace campvis {
    class ImageData;

    /**
     * VERY EXPERIMENTAL processor for manual segmentation of tissue layers.
     */
    class ManualTissueSegmenter : public VisualizationProcessor, public tgt::EventListener {
    public:
        /**
         * Constructs a new ManualTissueSegmenter Processor
         **/
        ManualTissueSegmenter(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~ManualTissueSegmenter();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractEventHandler::execute()
        virtual void onEvent(tgt::Event* e);

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "ManualTissueSegmenter"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "VERY EXPERIMENTAL processor for manual segmentation of tissue layers."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        DataNameProperty p_sourceImageID;                  ///< image ID for input FileIO
        DataNameProperty p_targetImageID;                  ///< image ID for output image

        IntProperty p_frameNumber;                          ///< Number of the frame to display
        TransferFunctionProperty p_transferFunction;        ///< Transfer function

        BoolProperty p_fullWidth;
        IntProperty p_splineOrder;
        IntProperty p_NumControlPoints;
        IntProperty p_NumLevels;

        ButtonProperty p_computeSamples;
        BoolProperty p_showSamples;
        StringProperty p_sampleFile;
        ButtonProperty p_saveSamples;

        StringProperty p_controlpointFile;
        ButtonProperty p_saveCPs;
        ButtonProperty p_loadCPs;

        IntProperty p_activeLayer;
        ButtonProperty p_addLayer;

        ButtonProperty p_exportToLabelImage;

    protected:
        struct ControlPoint {
            tgt::vec2 _pixel;
        };

        /// \see AbstractProcessor::updateResult()
        virtual void updateResult(DataContainer& data);

        /// adapts the range of the p_frameNumber property to the image
        virtual void updateProperties(DataContainer& data);

        /// \see AbstractProcessor::onPropertyChanged
        virtual void onPropertyChanged(const AbstractProperty* prop);

        void computeSpline();

        void computeSamples();

        void saveSamples();

        void saveControlPoints();

        void loadControlPoints();

        void onAddLayer();

        tgt::Shader* _shader;                           ///< Shader for slice rendering

        bool _mousePressed;                 ///< Flag whether the mouse is currently pressed
        tgt::ivec2 _mouseDownPosition;      ///< Viewport coordinates where mouse button has been pressed
        ControlPoint* _currentControlPoint;
        tgt::vec4* _currentBounds;

        DataHandle _currentImage;
        std::vector< std::map< int, std::vector< ControlPoint > > > _controlPoints;
        std::map< int, std::vector< float > > _splines;

        std::map< int, tgt::vec4 > _vesselBounds;

        TissueSegmentation _segmentation;

        static const std::string loggerCat_;
    };
}

#endif // MANUALTISSUESEGMENTER_H__

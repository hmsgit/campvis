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

#ifndef EEPGENERATOR_H__
#define EEPGENERATOR_H__

#include <string>

#include "core/classification/abstracttransferfunction.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/pipeline/abstractprocessordecorator.h"
#include "core/properties/cameraproperty.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/transferfunctionproperty.h"

namespace tgt {
    class Shader;
}

namespace campvis {
    /**
     * Genereates entry-/exit point textures for the given image and camera.
     */
    class EEPGenerator : public VisualizationProcessor, public HasProcessorDecorators {
    public:
        /**
         * Constructs a new EEPGenerator Processor
         **/
        EEPGenerator(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~EEPGenerator();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "EEPGenerator"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Genereates entry-/exit point textures for the given image and camera."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::TESTING; };

        virtual void process(DataContainer& data);

        DataNameProperty p_sourceImageID;    ///< image ID for input image
        DataNameProperty p_geometryID;       ///< ID for input geometry
        DataNameProperty p_geometryImageId;  ///< image ID for the optional rendered geometry to integrate into the EEP
        DataNameProperty p_entryImageID;     ///< image ID for output entry points image
        DataNameProperty p_exitImageID;      ///< image ID for output exit points image

        CameraProperty p_camera;

        BoolProperty p_enableMirror;         ///< Enable Virtual Mirror Feature
        DataNameProperty p_mirrorID;         ///< ID for input mirror geometry


    protected:
        /// \see HasProperyCollection::updateProperties()
        virtual void updateProperties();

        /**
         * \see RaycastingProcessor::generateHeader()
         * \return  "#define APPLY_MASK 1" if \a _applyMask is set to true.
         */
        virtual std::string generateHeader() const;

        tgt::Shader* _shader;                           ///< Shader for EEP generation

        static const std::string loggerCat_;
    };

}

#endif // EEPGENERATOR_H__

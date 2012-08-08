#ifndef EEPGENERATOR_H__
#define EEPGENERATOR_H__

#include <string>

#include "core/classification/abstracttransferfunction.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/cameraproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/transferfunctionproperty.h"

namespace tgt {
    class Shader;
}

namespace TUMVis {
    /**
     * Genereates entry-/exit point textures for the given image and camera.
     */
    class EEPGenerator : public VisualizationProcessor {
    public:
        /**
         * Constructs a new EEPGenerator Processor
         **/
        EEPGenerator(GenericProperty<tgt::ivec2>& canvasSize);

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

        virtual void process(DataContainer& data);

        GenericProperty<std::string> _sourceImageID;    ///< image ID for input image
        GenericProperty<std::string> _entryImageID;     ///< image ID for output entry points image
        GenericProperty<std::string> _exitImageID;      ///< image ID for output exit points image

        CameraProperty _camera;

    protected:

        tgt::Shader* _shader;                           ///< Shader for EEP generation

        static const std::string loggerCat_;
    };

}

#endif // EEPGENERATOR_H__

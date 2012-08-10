#ifndef DRRRAYCASTER_H__
#define DRRRAYCASTER_H__

#include <string>

#include "core/pipeline/raycastingprocessor.h"
#include "core/properties/genericproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/transferfunctionproperty.h"

namespace tgt {
    class Shader;
}

namespace TUMVis {
    class ImageData;

    /**
     * Creates a Digitally Reconstructed Radiograph.
     */
    class DRRRaycaster : public RaycastingProcessor {
    public:
        /**
         * Constructs a new DRRRaycaster Processor
         **/
        DRRRaycaster(GenericProperty<tgt::ivec2>& canvasSize);

        /**
         * Destructor
         **/
        virtual ~DRRRaycaster();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "DRRRaycaster"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Creates a Digitally Reconstructed Radiograph."; };

        GenericProperty<std::string> _targetImageID;    ///< image ID for output image

        FloatProperty _shift;
        FloatProperty _scale;
        BoolProperty _invertMapping;

    protected:
        /// \see RaycastingProcessor::processImpl()
        virtual void processImpl(DataContainer& data);

        /**
         * \see RaycastingProcessor::generateHeader()
         * \return  "#define DRR_INVERT 1" if \a _invertMapping is set to true.
         */
        virtual std::string generateHeader() const;

        static const std::string loggerCat_;
    };

}

#endif // DRRRAYCASTER_H__

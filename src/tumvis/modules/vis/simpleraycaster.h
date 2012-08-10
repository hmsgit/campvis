#ifndef SIMPLERAYCASTER_H__
#define SIMPLERAYCASTER_H__

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
     * Performs a simple volume ray casting.
     * \todo    OpenGL supports up to 4 bound FBO. We can use them to generate multiple images
     *          in a single pass, e.g. first hit point, normals, MIP, DVR.
     * \todo    Create some kind of RaycastingProcessor class to inherit from.
     */
    class SimpleRaycaster : public RaycastingProcessor {
    public:
        /**
         * Constructs a new SimpleRaycaster Processor
         **/
        SimpleRaycaster(GenericProperty<tgt::ivec2>& canvasSize);

        /**
         * Destructor
         **/
        virtual ~SimpleRaycaster();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "SimpleRaycaster"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Performs a simple volume ray casting."; };

        GenericProperty<std::string> _targetImageID;    ///< image ID for output image

    protected:
        /// \see RaycastingProcessor::processImpl()
        virtual void processImpl(DataContainer& data);

        static const std::string loggerCat_;
    };

}

#endif // SIMPLERAYCASTER_H__

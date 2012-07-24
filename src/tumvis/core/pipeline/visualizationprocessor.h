#ifndef VISUALIZATIONPROCESSOR_H__
#define VISUALIZATIONPROCESSOR_H__

#include "tgt/vector.h"
#include "core/pipeline/abstractprocessor.h"
#include "core/pipeline/visualizationpipeline.h"
#include "core/properties/genericproperty.h"

namespace TUMVis {

    /**
     * Specialization of AbstractProcessor for visualization purposes.
     * VisualizationProcessors are required to be called by a VisualizationPipeline which ensure
     * to provide a valid OpenGL context when calling the processor's process() method. Hence, a
     * VisualizationProcessor is allowed/capable of performing OpenGl operations.
     * For determining the canvas/viewport size, a VisualizationProcessor gets a reference to the
     * parent pipeline's render target size property during instantiation.
     * 
     * \sa VisualizationPipeline
     */
    class VisualizationProcessor : public AbstractProcessor {
    public:

        /**
         * Creates a VisualizationProcessor.
         * \note    The render target size property of this VisualizationProcessor will automatically 
         *          be assigned as shared property of the given \a renderTargetSize property.
         * \param   renderTargetSize    Reference to the parent pipeline's render target size property.
         */
        VisualizationProcessor(GenericProperty<tgt::ivec2>& renderTargetSize);

        /**
         * Virtual Destructor
         **/
        virtual ~VisualizationProcessor();


    protected:
        GenericProperty<tgt::ivec2> _renderTargetSize;        ///< Viewport size of target canvas
    };

}

#endif // VISUALIZATIONPROCESSOR_H__

#ifndef VISUALIZATIONPROCESSOR_H__
#define VISUALIZATIONPROCESSOR_H__

#include "tgt/vector.h"
#include "core/pipeline/abstractprocessor.h"
#include "core/pipeline/visualizationpipeline.h"
#include "core/properties/genericproperty.h"

namespace TUMVis {

    /**
     * Abstract base class for TUMVis Processors.
     * A processor implements a specific task, which it performs on the DataCollection passed
     * during process(). Properties provide a transparent layer for adjusting the processor's 
     * behaviour.
     * Once a processor has finished it sets it should set its invalidation level to valid. As
     * soon as one of its properties changes, the processor will be notified and possibliy
     * change its invalidation level. Observing pipelines will be notified of this and can
     * and have to decide which part of the pipeline has to be re-evaluated wrt. the processor's
     * invalidation level.
     * 
     * \sa AbstractPipeline
     */
    class VisualizationProcessor : public AbstractProcessor {
    public:

        /**
         * Creates a VisualizationProcessor.
         */
        VisualizationProcessor(GenericProperty<tgt::ivec2>& canvasSize);

        /**
         * Virtual Destructor
         **/
        virtual ~VisualizationProcessor();


    protected:
        GenericProperty<tgt::ivec2> _canvasSize;        ///< Viewport size of target canvas
    };

}

#endif // VISUALIZATIONPROCESSOR_H__

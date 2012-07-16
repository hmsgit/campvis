#ifndef SLICEVIS_H__
#define SLICEVIS_H__

#include "core/pipeline/visualizationpipeline.h"

namespace TUMVis {
    class SliceVis : public VisualizationPipeline {
    public:
        /**
         * Creates a VisualizationPipeline.
         */
        SliceVis(tgt::GLCanvas* canvas = 0);

        /**
         * Virtual Destructor
         **/
        virtual ~SliceVis();


        /**
         * Execute this pipeline.
         **/
        virtual void execute();

    };
}

#endif // SLICEVIS_H__

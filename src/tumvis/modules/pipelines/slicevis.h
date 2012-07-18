#ifndef SLICEVIS_H__
#define SLICEVIS_H__

#include "core/datastructures/imagedatalocal.h"
#include "core/pipeline/visualizationpipeline.h"
#include "modules/io/mhdimagereader.h"
#include "modules/vis/sliceextractor.h"

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

        /// \see VisualizationPipeline::init
        virtual void init();

        /**
         * Execute this pipeline.
         **/
        virtual void execute();

        virtual void keyEvent(tgt::KeyEvent* e);

    protected:
        MhdImageReader* _imageReader;
        SliceExtractor* _sliceExtractor;

    };
}

#endif // SLICEVIS_H__


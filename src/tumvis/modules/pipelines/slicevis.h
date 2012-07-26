#ifndef SLICEVIS_H__
#define SLICEVIS_H__

#include "core/datastructures/imagedatalocal.h"
#include "core/eventhandlers/mwheeltonumericpropertyeventhandler.h"
#include "core/pipeline/visualizationpipeline.h"
#include "modules/io/mhdimagereader.h"
#include "modules/vis/sliceextractor.h"

namespace TUMVis {
    class SliceVis : public VisualizationPipeline {
    public:
        /**
         * Creates a VisualizationPipeline.
         * \param   canvas  Canvas hosting the OpenGL context for this pipeline.
         */
        SliceVis();

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
        MhdImageReader _imageReader;
        SliceExtractor _sliceExtractor;

        MWheelToNumericPropertyEventHandler _wheelHandler;

    };
}

#endif // SLICEVIS_H__


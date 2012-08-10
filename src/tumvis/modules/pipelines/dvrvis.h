#ifndef DVRVIS_H__
#define DVRVIS_H__

#include "core/datastructures/imagedatalocal.h"
#include "core/eventhandlers/trackballnavigationeventhandler.h"
#include "core/pipeline/visualizationpipeline.h"
#include "modules/io/mhdimagereader.h"
#include "modules/vis/eepgenerator.h"
#include "modules/vis/drrraycaster.h"
#include "modules/vis/simpleraycaster.h"

namespace TUMVis {
    class DVRVis : public VisualizationPipeline {
    public:
        /**
         * Creates a VisualizationPipeline.
         */
        DVRVis();

        /**
         * Virtual Destructor
         **/
        virtual ~DVRVis();

        /// \see VisualizationPipeline::init()
        virtual void init();

        /// \see AbstractPipeline::getName()
        virtual const std::string getName() const;

        /**
         * Execute this pipeline.
         **/
        virtual void execute();

    protected:
        MhdImageReader _imageReader;
        EEPGenerator _eepGenerator;
        DRRRaycaster _drrraycater;
        SimpleRaycaster _simpleRaycaster;
        TrackballNavigationEventHandler* _trackballEH;

    };
}

#endif // DVRVIS_H__

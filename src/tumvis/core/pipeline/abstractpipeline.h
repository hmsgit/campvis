#ifndef ABSTRACTPIPELINE_H__
#define ABSTRACTPIPELINE_H__

#include "tgt/logmanager.h"
#include "core/datastructures/datacontainer.h"
#include "core/pipeline/abstractprocessor.h"
#include "core/properties/propertycollection.h"

#include <vector>

namespace TUMVis {

    /**
     * Abstract base class for TUMVis Pipelines.
     * 
     */
    class AbstractPipeline {
    public:
        /**
         * Creates a AbstractPipeline.
         */
        AbstractPipeline();

        /**
         * Virtual Destructor
         **/
        virtual ~AbstractPipeline();


        /**
         * Initializes the OpenGL context of the pipeline and its processors.
         * 
         * \note    When overwriting this method, make sure to call the base class version first.
         */
        virtual void init();

        /**
         * Execute this pipeline.
         **/
        virtual void execute() = 0;

        /**
         * Returns the PropertyCollection of this processor.
         * \return _properties
         */
        PropertyCollection& getPropertyCollection();

    protected:
        DataContainer _data;                                ///< DataContainer containing local working set of data for this Pipeline

        std::vector<AbstractProcessor*> _processors;        ///< List of all processors of this pipeline
        PropertyCollection _properties;                     ///< PropertyCollection of this pipeline, put in here all properties you want to be publicly accessible

        static const std::string loggerCat_;
    };

}


#endif // ABSTRACTPIPELINE_H__

#ifndef ABSTRACTPIPELINE_H__
#define ABSTRACTPIPELINE_H__

#include "tgt/logmanager.h"
#include "core/datastructures/datacontainer.h"

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
         * Execute this processor.
         **/
        virtual void execute() = 0;


    protected:
        DataContainer _data;                    ///< DataContainer containing local working set of data for this Pipeline

        static const std::string loggerCat_;
    };

}


#endif // ABSTRACTPIPELINE_H__

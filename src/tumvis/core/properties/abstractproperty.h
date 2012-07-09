#ifndef ABSTRACTPROPERTY_H__
#define ABSTRACTPROPERTY_H__

#include "tgt/logmanager.h"

#include <vector>

namespace TUMVis {

    /**
     * Abstract base class for TUMVis Pipelines.
     * 
     */
    class AbstractProperty {
    public:
        /**
         * Creates a AbstractProperty.
         */
        AbstractProperty();

        /**
         * Virtual Destructor
         **/
        virtual ~AbstractProperty();


    protected:

        static const std::string loggerCat_;
    };

}

#endif // ABSTRACTPROPERTY_H__

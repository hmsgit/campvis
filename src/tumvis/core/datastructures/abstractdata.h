#ifndef ABSTRACTDATA_H__
#define ABSTRACTDATA_H__

#include "core/tools/referencecounted.h"

namespace TUMVis {
    /**
     * Abstract base class for data handled by a DataHandle and stored in a DataContainer.
     * 
     * \todo 
     */
    class AbstractData : public ReferenceCounted {
    public:
        AbstractData();

        virtual ~AbstractData();

        virtual AbstractData* clone() const = 0;

    protected:

    };

}

#endif // ABSTRACTDATA_H__
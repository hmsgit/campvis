#ifndef abstractdata_h__
#define abstractdata_h__

#include <set>

namespace TUMVis {

    class DataHandle;

    /**
     * Abstract base class for data handled by a DataHandle and stored in a DataContainer.
     * 
     * \todo 
     */
    class AbstractData {
    public:
        AbstractData() {};
        virtual ~AbstractData() {};

        virtual AbstractData* clone() const = 0;

    protected:

    };

}

#endif // abstractdata_h__

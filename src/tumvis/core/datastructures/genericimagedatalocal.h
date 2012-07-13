#ifndef GENERICIMAGEDATALOCAL_H__
#define GENERICIMAGEDATALOCAL_H__

#include "tgt/vector.h"
#include "core/datastructures/imagedatalocal.h"

#include "core/tools/endianhelper.h"
#include "core/tools/typetraits.h"
#include "core/tools/weaklytypedpointer.h"

#include <cstring>
#include <fstream>
#include <string>

namespace TUMVis {

    template<typename BASETYPE, size_t NUMCHANNELS>
    class GenericImageDataLocal : public ImageDataLocal {
    public:
        typedef typedef TypeTraits<

        GenericImageDataLocal(size_t dimensionality, const tgt::svec3& size, T* data);

        virtual ~GenericImageDataLocal();


        /// \see AbstractData::clone()
        virtual ImageDataLocal<typename BASETYPE, size_t NUMCHANNELS>* clone() const;

        /// \see ImageData::getSubImage
        virtual ImageDataLocal<typename BASETYPE, size_t NUMCHANNELS>* getSubImage(const tgt::svec3& llf, const tgt::svec3& urb) const;
    protected:


    };

// - Template implementation ----------------------------------------------------------------------


}

#endif // GENERICIMAGEDATALOCAL_H__

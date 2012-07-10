#ifndef MHDIMAGEREADER_H__
#define MHDIMAGEREADER_H__

#include <string>

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/genericproperty.h"

namespace TUMVis {
    /**
     * 
     *
     * \note    Full format specification at http://www.itk.org/Wiki/MetaIO/Documentation
     */
    class MhdImageReader : public AbstractProcessor {
    public:
        /**
         * Constructs a new MhdImageReader Processor
         **/
        MhdImageReader();

        /**
         * Destructor
         **/
        virtual ~MhdImageReader();


        virtual void process();

    private:
        void parseHeader();

        // TODO make this a property as soon as the property system exists
        GenericProperty<std::string> _url;

        static const std::string loggerCat_;
    };

}

#endif // MHDIMAGEREADER_H__
#ifndef MHDIMAGEREADER_H__
#define MHDIMAGEREADER_H__

#include <string>

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/genericproperty.h"

namespace TUMVis {
    /**
     * Reads a MHD image file into the pipeline.
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


        /**
         * Reads the MHD file into an ImageDataDisk representation
         * \param data  DataContainer to work on
         */
        virtual void process(DataContainer& data);

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "MhdImageReader"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Reads an MHD image into the pipeline."; };

        GenericProperty<std::string> _url;              ///< URL for file to read
        GenericProperty<std::string> _targetImageID;    ///< image ID for read image

    protected:

        static const std::string loggerCat_;
    };

}

#endif // MHDIMAGEREADER_H__
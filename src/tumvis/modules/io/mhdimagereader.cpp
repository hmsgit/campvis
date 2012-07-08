#include "mhdimagereader.h"

#include <fstream>

#include "tgt/filesystem.h"
#include "core/datastructures/imagedatadisk.h"
#include "core/tools/textfileparser.h"

/*
 * Full format specification at http://www.itk.org/Wiki/MetaIO/Documentation
 */

namespace TUMVis {
    const std::string AbstractProcessor::loggerCat_ = "TUMVis.modules.io.MhdImageReader";

    MhdImageReader::MhdImageReader() 
        : AbstractProcessor()
    {

    }

    MhdImageReader::~MhdImageReader() {

    }

    void MhdImageReader::process() {
        TextFileParser tfp(_url, true, "=");
        tfp.parse<TextFileParser::ItemSeparatorLines>();

        std::string url;
        size_t dimensionality;
        tgt::svec3 size;
        WeaklyTypedPointer::PointerType pt;
        size_t offset = 0;
        EndianHelper::Endianness e = EndianHelper::LITTLE_ENDIAN;

        // start parsing
        try {
            // image type
            if (tfp.getString("ObjectType") != "Image") {
                LERROR("Error while parsing MHD header: ObjectType = Image expected");
                return;
            }

            // dimensionality and size
            dimensionality = tfp.getSizeT("NDims");
            if (dimensionality == 2)
                size = tgt::svec3(tfp.getSvec2("DimSize"), 0);
            else if (dimensionality == 3)
                size = tfp.getSvec3("DimSize");
            else {
                LERROR("Error while parsing MHD header: Unsupported dimensionality: " << dimensionality);
                return;
            }

            // element type
            std::string et = tfp.getString("ElementType");
            if (et == "MET_UCHAR")
                pt = WeaklyTypedPointer::UCHAR;
            else if (et == "MET_CHAR")
                pt = WeaklyTypedPointer::CHAR;
            else if (et == "MET_USHORT")
                pt = WeaklyTypedPointer::USHORT;
            else if (et == "MET_SHORT")
                pt = WeaklyTypedPointer::SHORT;
            else if (et == "MET_UINT")
                pt = WeaklyTypedPointer::UINT;
            else if (et == "MET_INT")
                pt = WeaklyTypedPointer::INT;
            else if (et == "MET_FLOAT")
                pt = WeaklyTypedPointer::FLOAT;
            else if (et == "MET_DOUBLE")
                pt = WeaklyTypedPointer::DOUBLE;
            else {
                LERROR("Error while parsing MHD header: Unsupported element type: " << et);
                return;
            }

            // further optional parameters:
            if (tfp.hasKey("HeaderSize")) {
                // header size can be -1...
                int tmp = tfp.getInt("HeaderSize");
                if (tmp >= 0)
                    offset = static_cast<int>(tmp);
            }
            if (tfp.hasKey("ElementByteOrderMSB"))
                e = (tfp.getBool("ElementByteOrderMSB") ? EndianHelper::BIG_ENDIAN : EndianHelper::LITTLE_ENDIAN);
            
            // TODO: spacing, element size, etc.


            // get raw image location:
            url = tfp.getString("ElementDataFile");
            if (url == "LOCAL") {
                url = _url;
                // find beginning of local data:
                tgt::File* file = FileSys.open(_url);
                if (!file || !file->isOpen())
                    throw tgt::FileException("Could not open file " + _url + " for reading.", _url);

                while (!file->eof()) {
                    std::string line = StringUtils::trim(file->getLine());
                    if (line.find("ElementDataFile") == 0) {
                        offset = file->tell();
                    }
                file->close();
                delete file;
                }
            }
            else if (url == "LIST") {
                LERROR("Error while loading MHD file: Image list currently not supported.");
                return;
            }


            // all parsing done - lets create the image:
            ImageDataDisk* image = new ImageDataDisk(url, dimensionality, size, pt, offset, e);
            _data.addData("output.image.read", image);
        }
        catch (tgt::Exception& e) {
            LERROR("Error while parsing MHD header: " << e.what());
            return;
        }
        catch (std::exception& e) {
            LERROR("Error while parsing MHD header: " << e.what());
            return;
        }

    }
}
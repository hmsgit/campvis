#include "mhdimagereader.h"

#include <fstream>

#include "tgt/filesystem.h"
#include "core/datastructures/imagedatadisk.h"
#include "core/datastructures/genericimagedatalocal.h"
#include "core/tools/textfileparser.h"

/*
 * Full format specification at http://www.itk.org/Wiki/MetaIO/Documentation
 */

namespace TUMVis {
    const std::string MhdImageReader::loggerCat_ = "TUMVis.modules.io.MhdImageReader";

    MhdImageReader::MhdImageReader() 
        : AbstractProcessor()
        , _url("url", "Image URL", "")
        , _targetImageID("targetImageName", "Target Image ID", "MhdImageReader.output")
    {
        _properties.addProperty(&_url);
        _url.addObserver(this);
        _properties.addProperty(&_targetImageID);
        _targetImageID.addObserver(this);
    }

    MhdImageReader::~MhdImageReader() {

    }

    void MhdImageReader::process(DataContainer& data) {
        try {
            // start parsing
            TextFileParser tfp(_url.getValue(), true, "=");
            tfp.parse<TextFileParser::ItemSeparatorLines>();

            // init optional parameters with sane default values
            std::string url;
            size_t dimensionality;
            tgt::svec3 size;
            WeaklyTypedPointer::BaseType pt;
            size_t offset = 0;
            EndianHelper::Endianness e = EndianHelper::LITTLE_ENDIAN;

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
                pt = WeaklyTypedPointer::UINT8;
            else if (et == "MET_CHAR")
                pt = WeaklyTypedPointer::INT8;
            else if (et == "MET_USHORT")
                pt = WeaklyTypedPointer::UINT16;
            else if (et == "MET_SHORT")
                pt = WeaklyTypedPointer::INT16;
            else if (et == "MET_UINT")
                pt = WeaklyTypedPointer::UINT32;
            else if (et == "MET_INT")
                pt = WeaklyTypedPointer::INT32;
            else if (et == "MET_FLOAT")
                pt = WeaklyTypedPointer::FLOAT;
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
            url = StringUtils::trim(tfp.getString("ElementDataFile"));
            if (url == "LOCAL") {
                url = _url.getValue();
                // find beginning of local data:
                tgt::File* file = FileSys.open(_url.getValue());
                if (!file || !file->isOpen())
                    throw tgt::FileException("Could not open file " + _url.getValue() + " for reading.", _url.getValue());

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
            else {
                url = tgt::FileSystem::cleanupPath(tgt::FileSystem::dirName(_url.getValue()) + "/" + url);
            } 



            // all parsing done - lets create the image:
            ImageDataDisk* image = new ImageDataDisk(url, dimensionality, size, pt, 1, offset, e);
            data.addData(_targetImageID.getValue(), image);
        }
        catch (tgt::Exception& e) {
            LERROR("Error while parsing MHD header: " << e.what());
            return;
        }
        catch (std::exception& e) {
            LERROR("Error while parsing MHD header: " << e.what());
            return;
        }

        _invalidationLevel.setValid();
    }
}
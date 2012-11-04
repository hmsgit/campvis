// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#include "mhdimagereader.h"

#include <fstream>

#include "tgt/filesystem.h"
#include "core/datastructures/imagedatadisk.h"
#include "core/datastructures/genericimagedatalocal.h"
#include "core/tools/textfileparser.h"

/*
 * Full format specification at http://www.itk.org/Wiki/MetaIO/Documentation
 */

namespace campvis {
    const std::string MhdImageReader::loggerCat_ = "CAMPVis.modules.io.MhdImageReader";

    MhdImageReader::MhdImageReader() 
        : AbstractProcessor()
        , p_url("url", "Image URL", "")
        , p_targetImageID("targetImageName", "Target Image ID", "MhdImageReader.output", DataNameProperty::WRITE)
    {
        addProperty(&p_url);
        addProperty(&p_targetImageID);
    }

    MhdImageReader::~MhdImageReader() {

    }

    void MhdImageReader::process(DataContainer& data) {
        try {
            // start parsing
            TextFileParser tfp(p_url.getValue(), true, "=");
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
                url = p_url.getValue();
                // find beginning of local data:
                tgt::File* file = FileSys.open(p_url.getValue());
                if (!file || !file->isOpen())
                    throw tgt::FileException("Could not open file " + p_url.getValue() + " for reading.", p_url.getValue());

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
                url = tgt::FileSystem::cleanupPath(tgt::FileSystem::dirName(p_url.getValue()) + "/" + url);
            } 



            // all parsing done - lets create the image:
            ImageDataDisk* image = new ImageDataDisk(url, dimensionality, size, pt, 1, offset, e);
            data.addData(p_targetImageID.getValue(), image);
            p_targetImageID.issueWrite();
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
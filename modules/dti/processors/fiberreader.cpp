// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
// 
// ================================================================================================

#include "fiberreader.h"

#include "cgt/filesystem.h"
#include "modules/dti/datastructures/fiberdata.h"
#include <fstream>

namespace {

    /// File header for TrackVis *.trk files
    struct trk_header {
        //                                   SIZE  DESCRIPTION
        char id_string[6];                   //6   ID string for track file. The first 5 characters must be "TRACK".
        short int dim[3];                    //6   Dimension of the image volume.
        float voxel_size[3];                 //12  Voxel size of the image volume.
        float origin[3];                     //12  Origin of the image volume. This field is not yet being used by TrackVis. That means the origin is always (0, 0, 0).
        short int n_scalars;                 //2   Number of scalars saved at each track point (besides x, y and z coordinates).
        char scalar_name[10][20];            //200 Name of each scalar. Can not be longer than 20 characters each. Can only store up to 10 names.
        short int n_properties;              //2   Number of properties saved at each track.
        char property_name[10][20];          //200 Name of each property. Can not be longer than 20 characters each. Can only store up to 10 names.
        float vox_to_ras[4][4];              //64  4x4 matrix for voxel to RAS (crs to xyz) transformation.
                                             //    If vox_to_ras[3][3] is 0, it means the matrix is not recorded.
                                             //    This field is added from version 2.
        char reserved[444];                  //444 Reserved space for future version.
        char voxel_order[4];                 //4   Storing order of the original image data. Explained here.
        char pad2[4];                        //4   Paddings.
        float image_orientation_patient[6];  //24  Image orientation of the original image. As defined in the DICOM header.
        char pad1[2];                        //2   Paddings.
        unsigned char invert_x;              //1   Inversion/rotation flags used to generate this track file. For internal use only.
        unsigned char invert_y;              //1   As above.
        unsigned char invert_z;              //1   As above.
        unsigned char swap_xy;               //1   As above.
        unsigned char swap_yz;               //1   As above.
        unsigned char swap_zx;               //1   As above.
        int32_t n_count;                     //4   Number of tracks stored in this track file. 0 means the number was NOT stored.
        int32_t version;                     //4   Version number. Current version is 2.
        int32_t hdr_size;                    //4   Size of the header. Used to determine byte swap. Should be 1000.
    };

}

namespace campvis {
namespace dti {

    const std::string FiberReader::loggerCat_ = "CAMPVis.modules.dti.FiberReader";

    FiberReader::FiberReader()
        : p_url("Url", "Input File Name", "", StringProperty::OPEN_FILENAME)
        , p_outputId("OutputId", "Output Fiber Data Name", "fibers", DataNameProperty::WRITE)
        , p_offset("Offset", "Additional Offset in mm", cgt::vec3(0.f), cgt::vec3(-10000.f), cgt::vec3(10000.f), cgt::vec3(0.1f))
        , p_scaling("Scaling", "Additional Scaling", cgt::vec3(1.f), cgt::vec3(-100.f), cgt::vec3(100.f), cgt::vec3(0.1f))
    {
        addProperty(p_url);
        addProperty(p_outputId);
        addProperty(p_offset);
        addProperty(p_scaling);
    }

    FiberReader::~FiberReader() {

    }

    void FiberReader::updateResult(DataContainer& dataContainer) {
        const std::string& fileName = p_url.getValue();
        if (cgt::FileSystem::fileExtension(fileName) == "trk") {
            dataContainer.addData(p_outputId.getValue(), readTrkFile(fileName));
        }
        else {
            LERROR("Unknown file extension.");
        }
    }

    FiberData* FiberReader::readTrkFile(const std::string& fileName) {
        cgtAssert(sizeof(trk_header) == 1000, "invalid trk header size!");
        trk_header header;
        FiberData* toReturn = nullptr;

        std::ifstream file(fileName, std::ios::in |std::ios::binary);
        if (!file) {
            LERROR("Failed to open file: " << fileName);
            return nullptr;
        }
        if (!file.read((char*)&header, sizeof(header))) {
            LERROR("Failed to read header! File: " << fileName);
            return nullptr;
        }

        toReturn = new FiberData();
        int32_t numPoints;

        //LINFO("n_scalars: " << header.n_scalars);
        //LINFO("n_properties: " << header.n_properties);
        float* scalars = new float[header.n_scalars];
        float* properties = new float[header.n_properties];

        while (file.read(reinterpret_cast<char*>(&numPoints), 4)) {
            std::vector<cgt::vec3> vertices;
            vertices.resize(numPoints, cgt::vec3(0.f));


            for (int32_t i=0; i<numPoints; ++i) {
                file.read(reinterpret_cast<char*>(vertices[i].elem), sizeof(cgt::vec3));

                vertices[i] *= p_scaling.getValue();
                vertices[i] += p_offset.getValue();

                file.read((char*)scalars, sizeof(float) * header.n_scalars);
                //if(header.n_scalars > 0)
                //    fibers_.uncertainties_.push_back(scalars[0]);
                //else
                //    fibers_.uncertainties_.push_back(0.0f);
            }

            toReturn->addFiber(vertices);
            file.read((char*) properties, sizeof(float) * header.n_properties);
        }

        delete[] scalars;
        delete[] properties;
        file.close();

        return toReturn;
    }

}
}

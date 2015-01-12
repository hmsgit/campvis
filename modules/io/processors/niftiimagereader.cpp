// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
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

#include "niftiimagereader.h"

#include <fstream>

#include "cgt/filesystem.h"
#include "cgt/vector.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imageseries.h"
#include "core/datastructures/imagerepresentationdisk.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/tools/endianhelper.h"
#include "core/tools/textfileparser.h"

/*
 * Full format specification at http://brainder.org/2012/09/23/the-nifti-file-format/
 * Implementation heavily influenced by Voreen's AnalyzeVolumeReader.
 */

namespace {
#define swap_long campvis::EndianHelper::swapEndian<4> 
#define swap_short campvis::EndianHelper::swapEndian<2> 

struct header_key {
    int32_t sizeof_hdr;
    char data_type[10];
    char db_name[18];
    int32_t extents;
    int16_t session_error;
    char regular;
    char hkey_un0;

    void swapEndianess() {
        swap_long(reinterpret_cast<char*>(&sizeof_hdr));
        swap_long(reinterpret_cast<char*>(&extents));
        swap_short(reinterpret_cast<char*>(&session_error));
    }
};
/* 40 bytes */

struct image_dimension
{
    int16_t dim[8];
    int16_t unused8;
    int16_t unused9;
    int16_t unused10;
    int16_t unused11;
    int16_t unused12;
    int16_t unused13;
    int16_t unused14;
    int16_t datatype;
    int16_t bitpix;
    int16_t dim_un0;
    float pixdim[8];
    float vox_offset;
    float funused1;
    float funused2;
    float funused3;
    float cal_max;
    float cal_min;
    float compressed;
    float verified;
    int32_t glmax,glmin;

    void swapEndianess() {
        swap_short(reinterpret_cast<char*>(&dim[0]));
        swap_short(reinterpret_cast<char*>(&dim[1]));
        swap_short(reinterpret_cast<char*>(&dim[2]));
        swap_short(reinterpret_cast<char*>(&dim[3]));
        swap_short(reinterpret_cast<char*>(&dim[4]));
        swap_short(reinterpret_cast<char*>(&dim[5]));
        swap_short(reinterpret_cast<char*>(&dim[6]));
        swap_short(reinterpret_cast<char*>(&dim[7]));
        swap_short(reinterpret_cast<char*>(&unused8));
        swap_short(reinterpret_cast<char*>(&unused9));
        swap_short(reinterpret_cast<char*>(&unused10));
        swap_short(reinterpret_cast<char*>(&unused11));
        swap_short(reinterpret_cast<char*>(&unused12));
        swap_short(reinterpret_cast<char*>(&unused13));
        swap_short(reinterpret_cast<char*>(&unused14));
        swap_short(reinterpret_cast<char*>(&datatype));
        swap_short(reinterpret_cast<char*>(&bitpix));
        swap_long(reinterpret_cast<char*>(&pixdim[0]));
        swap_long(reinterpret_cast<char*>(&pixdim[1]));
        swap_long(reinterpret_cast<char*>(&pixdim[2]));
        swap_long(reinterpret_cast<char*>(&pixdim[3]));
        swap_long(reinterpret_cast<char*>(&pixdim[4]));
        swap_long(reinterpret_cast<char*>(&pixdim[5]));
        swap_long(reinterpret_cast<char*>(&pixdim[5]));
        swap_long(reinterpret_cast<char*>(&pixdim[6]));
        swap_long(reinterpret_cast<char*>(&pixdim[7]));
        swap_long(reinterpret_cast<char*>(&vox_offset));
        swap_long(reinterpret_cast<char*>(&funused1));
        swap_long(reinterpret_cast<char*>(&funused2));
        swap_long(reinterpret_cast<char*>(&cal_max));
        swap_long(reinterpret_cast<char*>(&cal_min));
        swap_long(reinterpret_cast<char*>(&compressed));
        swap_long(reinterpret_cast<char*>(&verified));
        swap_short(reinterpret_cast<char*>(&dim_un0));
        swap_long(reinterpret_cast<char*>(&glmax));
        swap_long(reinterpret_cast<char*>(&glmin));
    }
};
/* 108 bytes */

struct data_history
{
    char descrip[80];
    char aux_file[24];
    char orient;
    char originator[10];
    char generated[10];
    char scannum[10];
    char patient_id[10];
    char exp_date[10];
    char exp_time[10];
    char hist_un0[3];
    int32_t views;
    int32_t vols_added;
    int32_t start_field;
    int32_t field_skip;
    int32_t omax, omin;
    int32_t smax, smin;

    void swapEndianess() {
        swap_long(reinterpret_cast<char*>(&views));
        swap_long(reinterpret_cast<char*>(&vols_added));
        swap_long(reinterpret_cast<char*>(&start_field));
        swap_long(reinterpret_cast<char*>(&field_skip));
        swap_long(reinterpret_cast<char*>(&omax));
        swap_long(reinterpret_cast<char*>(&omin));
        swap_long(reinterpret_cast<char*>(&smax));
        swap_long(reinterpret_cast<char*>(&smin));
    }
};
/* 200 bytes */

struct analyze_header
{
    header_key hk;
    image_dimension dime;
    data_history hist;
};
/* 348 bytes */

/* Acceptable values for datatype */
#define DT_NONE 0
#define DT_UNKNOWN 0
#define DT_BINARY 1
#define DT_UNSIGNED_CHAR 2
#define DT_SIGNED_SHORT 4
#define DT_SIGNED_INT 8
#define DT_FLOAT 16
#define DT_COMPLEX 32
#define DT_DOUBLE 64
#define DT_RGB 128
#define DT_ALL 255

                            /*------------------- new codes for NIFTI ---*/
#define DT_INT8                  256     /* signed char (8 bits)         */
#define DT_UINT16                512     /* unsigned short (16 bits)     */
#define DT_UINT32                768     /* unsigned int (32 bits)       */
#define DT_INT64                1024     /* long long (64 bits)          */
#define DT_UINT64               1280     /* unsigned long long (64 bits) */
#define DT_FLOAT128             1536     /* long double (128 bits)       */
#define DT_COMPLEX128           1792     /* double pair (128 bits)       */
#define DT_COMPLEX256           2048     /* long double pair (256 bits)  */
#define DT_RGBA32               2304     /* 4 byte RGBA (32 bits/voxel)  */

/* Acceptable values for intent codes, see http://nifti.nimh.nih.gov/nifti-1/documentation/nifti1diagrams_v2.pdf */
#define IC_INTENT_NONE          0
#define IC_INTENT_ESTIMATE      1001    /* parameter for estimate in intent_name */
#define IC_INTENT_LABEL         1002    /* parameter at each voxel is index to label defined in aux_file */
#define IC_INTENT_NEURONAME     1003    /* parameter at each voxel is index to label in NeuroNames label set */
#define IC_INTENT_GENMATRIX     1004    /* parameter at each voxel is matrix */
#define IC_INTENT_SYMMATRIX     1005    /* parameter at each voxel is symmetrical matrix */
#define IC_INTENT_DISPVECT      1006    /* parameter at each voxel is displacement vector */
#define IC_INTENT_VECTOR        1007    /* parameter at each voxel is vector */
#define IC_INTENT_POINTSET      1008    /* value at each voxel is spatial coordinate (vertices/nodes of surface mesh) */
#define IC_INTENT_TRIANGLE      1009    /* value at each voxel is spatial coordinate (vertices/nodes of surface mesh) */
#define IC_INTENT_QUATERNION    1010
#define IC_INTENT_DIMLESS       1011    /* dimensionless value */

#define NIFTI_UNITS_UNKNOWN 0

/** Space codes are multiples of 1. **/
#define NIFTI_UNITS_METER   1 /*! NIFTI code for meters. */
#define NIFTI_UNITS_MM      2 /*! NIFTI code for millimeters. */
#define NIFTI_UNITS_MICRON  3 /*! NIFTI code for micrometers. */

/** Time codes are multiples of 8. **/
#define NIFTI_UNITS_SEC     8 /*! NIFTI code for seconds. */
#define NIFTI_UNITS_MSEC   16 /*! NIFTI code for milliseconds. */
#define NIFTI_UNITS_USEC   24 /*! NIFTI code for microseconds. */

/*** These units are for spectral data: ***/
#define NIFTI_UNITS_HZ     32 /*! NIFTI code for Hertz. */
#define NIFTI_UNITS_PPM    40 /*! NIFTI code for ppm. */
#define NIFTI_UNITS_RADS   48 /*! NIFTI code for radians per second. */

#define XYZT_TO_SPACE(xyzt)       ( (xyzt) & 0x07 )
#define XYZT_TO_TIME(xyzt)        ( (xyzt) & 0x38 )

//struct complex
//{
    //float real;
    //float imag;
//};

struct nifti_1_header { /* NIFTI-1 usage         */  /* ANALYZE 7.5 field(s) */
                        /*************************/  /************************/
                                           /*--- was header_key substruct ---*/
 int32_t   sizeof_hdr;    /*!< MUST be 348           */  /* int sizeof_hdr;      */
 char  data_type[10]; /*!< ++UNUSED++            */  /* char data_type[10];  */
 char  db_name[18];   /*!< ++UNUSED++            */  /* char db_name[18];    */
 int32_t   extents;       /*!< ++UNUSED++            */  /* int extents;         */
 int16_t session_error; /*!< ++UNUSED++            */  /* short session_error; */
 char  regular;       /*!< ++UNUSED++            */  /* char regular;        */
 char  dim_info;      /*!< MRI slice ordering.   */  /* char hkey_un0;       */

                                      /*--- was image_dimension substruct ---*/
 int16_t dim[8];        /*!< Data array dimensions.*/  /* short dim[8];        */
 float intent_p1 ;    /*!< 1st intent parameter. */  /* int16_t unused8;       */
                                                     /* int16_t unused9;       */
 float intent_p2 ;    /*!< 2nd intent parameter. */  /* int16_t unused10;      */
                                                     /* int16_t unused11;      */
 float intent_p3 ;    /*!< 3rd intent parameter. */  /* int16_t unused12;      */
                                                     /* int16_t unused13;      */
 int16_t intent_code ;  /*!< NIFTI_INTENT_* code.  */  /* short unused14;      */
 int16_t datatype;      /*!< Defines data type!    */  /* short datatype;      */
 int16_t bitpix;        /*!< Number bits/voxel.    */  /* short bitpix;        */
 int16_t slice_start;   /*!< First slice index.    */  /* short dim_un0;       */
 float pixdim[8];     /*!< Grid spacings.        */  /* float pixdim[8];     */
 float vox_offset;    /*!< Offset into .nii file */  /* float vox_offset;    */
 float scl_slope ;    /*!< Data scaling: slope.  */  /* float funused1;      */
 float scl_inter ;    /*!< Data scaling: offset. */  /* float funused2;      */
 int16_t slice_end;     /*!< Last slice index.     */  /* float funused3;      */
 char  slice_code ;   /*!< Slice timing order.   */
 char  xyzt_units ;   /*!< Units of pixdim[1..4] */
 float cal_max;       /*!< Max display intensity */  /* float cal_max;       */
 float cal_min;       /*!< Min display intensity */  /* float cal_min;       */
 float slice_duration;/*!< Time for 1 slice.     */  /* float compressed;    */
 float toffset;       /*!< Time axis shift.      */  /* float verified;      */
 int32_t   glmax;         /*!< ++UNUSED++            */  /* int glmax;           */
 int32_t   glmin;         /*!< ++UNUSED++            */  /* int glmin;           */

                                         /*--- was data_history substruct ---*/
 char  descrip[80];   /*!< any text you like.    */  /* char descrip[80];    */
 char  aux_file[24];  /*!< auxiliary filename.   */  /* char aux_file[24];   */

 int16_t qform_code ;   /*!< NIFTI_XFORM_* code.   */  /*-- all ANALYZE 7.5 ---*/
 int16_t sform_code ;   /*!< NIFTI_XFORM_* code.   */  /*   fields below here  */
                                                     /*   are replaced       */
 float quatern_b ;    /*!< Quaternion b param.   */
 float quatern_c ;    /*!< Quaternion c param.   */
 float quatern_d ;    /*!< Quaternion d param.   */
 float qoffset_x ;    /*!< Quaternion x shift.   */
 float qoffset_y ;    /*!< Quaternion y shift.   */
 float qoffset_z ;    /*!< Quaternion z shift.   */

 float srow_x[4] ;    /*!< 1st row affine transform.   */
 float srow_y[4] ;    /*!< 2nd row affine transform.   */
 float srow_z[4] ;    /*!< 3rd row affine transform.   */

 char intent_name[16];/*!< 'name' or meaning of data.  */

 char magic[4] ;      /*!< MUST be "ni1\0" or "n+1\0". */

 void swapEndianess() {
     swap_long(reinterpret_cast<char*>(&sizeof_hdr));
     swap_long(reinterpret_cast<char*>(&extents));
     swap_short(reinterpret_cast<char*>(&session_error));
     swap_short(reinterpret_cast<char*>(&dim[0]));
     swap_short(reinterpret_cast<char*>(&dim[1]));
     swap_short(reinterpret_cast<char*>(&dim[2]));
     swap_short(reinterpret_cast<char*>(&dim[3]));
     swap_short(reinterpret_cast<char*>(&dim[4]));
     swap_short(reinterpret_cast<char*>(&dim[5]));
     swap_short(reinterpret_cast<char*>(&dim[6]));
     swap_short(reinterpret_cast<char*>(&dim[7]));
     swap_long(reinterpret_cast<char*>(&intent_p1));
     swap_long(reinterpret_cast<char*>(&intent_p2));
     swap_long(reinterpret_cast<char*>(&intent_p3));
     swap_short(reinterpret_cast<char*>(&intent_code));
     swap_short(reinterpret_cast<char*>(&datatype));
     swap_short(reinterpret_cast<char*>(&bitpix));
     swap_short(reinterpret_cast<char*>(&slice_start));
     swap_long(reinterpret_cast<char*>(&pixdim[0]));
     swap_long(reinterpret_cast<char*>(&pixdim[1]));
     swap_long(reinterpret_cast<char*>(&pixdim[2]));
     swap_long(reinterpret_cast<char*>(&pixdim[3]));
     swap_long(reinterpret_cast<char*>(&pixdim[4]));
     swap_long(reinterpret_cast<char*>(&pixdim[5]));
     swap_long(reinterpret_cast<char*>(&pixdim[6]));
     swap_long(reinterpret_cast<char*>(&pixdim[7]));
     swap_long(reinterpret_cast<char*>(&vox_offset));
     swap_long(reinterpret_cast<char*>(&scl_slope));
     swap_long(reinterpret_cast<char*>(&scl_inter));
     swap_short(reinterpret_cast<char*>(&slice_end));
     swap_long(reinterpret_cast<char*>(&cal_max));
     swap_long(reinterpret_cast<char*>(&cal_min));
     swap_long(reinterpret_cast<char*>(&slice_duration));
     swap_long(reinterpret_cast<char*>(&toffset));
     swap_long(reinterpret_cast<char*>(&glmax));
     swap_long(reinterpret_cast<char*>(&glmin));
     swap_short(reinterpret_cast<char*>(&qform_code));
     swap_short(reinterpret_cast<char*>(&sform_code));
     swap_long(reinterpret_cast<char*>(&quatern_b));
     swap_long(reinterpret_cast<char*>(&quatern_c));
     swap_long(reinterpret_cast<char*>(&quatern_d));
     swap_long(reinterpret_cast<char*>(&qoffset_x));
     swap_long(reinterpret_cast<char*>(&qoffset_y));
     swap_long(reinterpret_cast<char*>(&qoffset_z));

     swap_long(reinterpret_cast<char*>(&srow_x[0]));
     swap_long(reinterpret_cast<char*>(&srow_x[1]));
     swap_long(reinterpret_cast<char*>(&srow_x[2]));
     swap_long(reinterpret_cast<char*>(&srow_x[3]));

     swap_long(reinterpret_cast<char*>(&srow_y[0]));
     swap_long(reinterpret_cast<char*>(&srow_y[1]));
     swap_long(reinterpret_cast<char*>(&srow_y[2]));
     swap_long(reinterpret_cast<char*>(&srow_y[3]));

     swap_long(reinterpret_cast<char*>(&srow_z[0]));
     swap_long(reinterpret_cast<char*>(&srow_z[1]));
     swap_long(reinterpret_cast<char*>(&srow_z[2]));
     swap_long(reinterpret_cast<char*>(&srow_z[3]));
 }
};                   /**** 348 bytes total ****/

}
namespace campvis {
    const std::string NiftiImageReader::loggerCat_ = "CAMPVis.modules.io.NiftiImageReader";

    NiftiImageReader::NiftiImageReader() 
        : AbstractImageReader()
        , p_imageOffset("ImageOffset", "Image Offset in mm", cgt::vec3(0.f), cgt::vec3(-10000.f), cgt::vec3(10000.f), cgt::vec3(0.1f))
        , p_voxelSize("VoxelSize", "Voxel Size in mm", cgt::vec3(1.f), cgt::vec3(-100.f), cgt::vec3(100.f), cgt::vec3(0.1f))
    {
        this->_ext.push_back("hdr");
        this->_ext.push_back("nii");
        this->p_targetImageID.setValue("NiftiImageReader.output");
        addProperty(p_url);
        addProperty(p_targetImageID);
        addProperty(p_imageOffset);
        addProperty(p_voxelSize);

        //make sure the header sizes are ok when compiled on all architectures:
        cgtAssert(sizeof(header_key) == 40, "Wrong header_key size!");
        cgtAssert(sizeof(image_dimension) == 108, "Wrong image_dimension size!");
        cgtAssert(sizeof(data_history) == 200, "Wrong data_history size!");
        cgtAssert(sizeof(analyze_header) == 348, "Wrong analyze_header size!");
        cgtAssert(sizeof(nifti_1_header) == 348, "Wrong nifti_1_header size!");
    }

    NiftiImageReader::~NiftiImageReader() {

    }

    void NiftiImageReader::updateResult(DataContainer& dataContainer) {
        try {
            const std::string& fileName = p_url.getValue();

            if (cgt::FileSystem::fileExtension(fileName) == "nii")
                readNifti(dataContainer, fileName, true);
            else {
                //check magic string:
                std::ifstream file(fileName.c_str(), std::ios::in | std::ios::binary);
                if (!file) {
                    throw cgt::FileNotFoundException("Failed to open file: ", fileName);
                }

                file.seekg(0, std::ios::end);
                std::streamoff fileSize = file.tellg();
                file.seekg(0, std::ios::beg);

                if (fileSize < 348)
                    throw cgt::CorruptedFileException("Analyze/Nifti file is smaller than 348 bytes!", fileName);

                nifti_1_header header;
                if (!file.read((char*)&header, sizeof(header))) {
                    throw cgt::CorruptedFileException("Failed to read header!", fileName);
                }
                file.close();

                if ((header.magic[0] == 'n') && (header.magic[2] == '1') && (header.magic[3] == 0)) {
                    if(header.magic[1] == '+') {
                        readNifti(dataContainer, fileName, true);
                    }
                    else if(header.magic[1] == 'i') {
                        readNifti(dataContainer, fileName, false);
                    }
                    else
                        readAnalyze(dataContainer, fileName);
                }
                else
                    readAnalyze(dataContainer, fileName);
            }
        }
        catch (cgt::Exception& e) {
            LERROR("Error while parsing MHD header: " << e.what());
            return;
        }
        catch (std::exception& e) {
            LERROR("Error while parsing MHD header: " << e.what());
            return;
        }
    }

    void NiftiImageReader::readAnalyze(DataContainer& dataContainer, const std::string& fileName) throw(cgt::FileException, std::bad_alloc) {
        std::string hdrFileName = cgt::FileSystem::fullBaseName(fileName) + ".hdr";

        std::ifstream file(fileName.c_str(), std::ios::in | std::ios::binary);
        if (!file) {
            throw cgt::FileNotFoundException("Failed to open file: ", fileName);
        }
        std::ifstream hdrFile(hdrFileName.c_str(), std::ios::in | std::ios::binary);
        if (!hdrFile) {
            throw cgt::FileNotFoundException("Failed to open file: ", hdrFileName);
        }

        header_key header;
        if (!file.read((char*)&header, sizeof(header))) {
            throw cgt::CorruptedFileException("Failed to read header!", fileName);
        }

        image_dimension dimension;
        if (!file.read((char*)&dimension, sizeof(dimension))) {
            throw cgt::CorruptedFileException("Failed to read dimensions!", fileName);
        }

        data_history history;
        if (!file.read((char*)&history, sizeof(history))) {
            throw cgt::CorruptedFileException("Failed to read history!", fileName);
        }

        EndianHelper::Endianness e = EndianHelper::IS_LITTLE_ENDIAN;
        //check if swap is necessary:
        if ((dimension.dim[0] < 0) || (dimension.dim[0] > 15)) {
            e = EndianHelper::IS_BIG_ENDIAN;
            header.swapEndianess();
            dimension.swapEndianess();
            history.swapEndianess();
        }

        cgt::ivec3 dimensions;
        dimensions.x = dimension.dim[1];
        dimensions.y = dimension.dim[2];
        dimensions.z = dimension.dim[3];

        size_t numVolumes = static_cast<size_t>(dimension.dim[4]);

        if (cgt::hor(cgt::lessThanEqual(dimensions, cgt::ivec3(0)))) {
            LERROR("Invalid resolution or resolution not specified: " << dimensions);
            throw cgt::CorruptedFileException("error while reading data", fileName);
        }

        cgt::vec3 spacing;
        spacing.x = dimension.pixdim[1];
        spacing.y = dimension.pixdim[2];
        spacing.z = dimension.pixdim[3];

        size_t numChannels = 1;
        WeaklyTypedPointer::BaseType baseType;
        switch(dimension.datatype) {
            case DT_UNSIGNED_CHAR:
                baseType = WeaklyTypedPointer::UINT8;
                break;
            case DT_SIGNED_SHORT:
                baseType = WeaklyTypedPointer::INT16;
                break;
            case DT_SIGNED_INT:
                baseType = WeaklyTypedPointer::INT32;
                break;
            case DT_FLOAT:
                baseType = WeaklyTypedPointer::FLOAT;
                break;
            case DT_RGB:
                baseType = WeaklyTypedPointer::UINT8;
                numChannels = 3;
                break;
            case DT_ALL:
            case DT_COMPLEX:
            case 0: //DT_NONE/DT_UNKNOWN
            case DT_BINARY:
            default:
                throw cgt::UnsupportedFormatException("Unsupported datatype!");
        }

        cgt::svec3 imageSize(dimensions);
        if (numVolumes <= 1) {
            ImageData* image = new ImageData(3, imageSize, numChannels);
            ImageRepresentationDisk::create(image, hdrFileName, baseType, 0, e);
            // Nifti transformations give us the center of the first voxel, we translate to correct:
            image->setMappingInformation(ImageMappingInformation(imageSize, cgt::vec3(-.5f) + p_imageOffset.getValue(), spacing * p_voxelSize.getValue()));
            dataContainer.addData(p_targetImageID.getValue(), image);
        }
        else {
            ImageSeries* is = new ImageSeries();
            size_t numBytes = hmul(imageSize) * (dimension.bitpix / 8);
            for (size_t i = 0; i < numVolumes; ++i) {
                ImageData* image = new ImageData(3, imageSize, numChannels);
                ImageRepresentationDisk::create(image, hdrFileName, baseType, i * numBytes, e);
                // Nifti transformations give us the center of the first voxel, we translate to correct:
                image->setMappingInformation(ImageMappingInformation(imageSize, cgt::vec3(-.5f) + p_imageOffset.getValue(), spacing * p_voxelSize.getValue()));
                is->addImage(image);
            }
            dataContainer.addData(p_targetImageID.getValue(), is);
        }
    }

    void NiftiImageReader::readNifti(DataContainer& dataContainer, const std::string& fileName, bool standalone) throw(cgt::FileException, std::bad_alloc) {
        std::ifstream file(fileName.c_str(), std::ios::in | std::ios::binary);
        if (!file) {
            throw cgt::FileNotFoundException("Failed to open file: ", fileName);
        }

        nifti_1_header header;
        if (!file.read((char*)&header, sizeof(header))) {
            throw cgt::CorruptedFileException("Failed to read header!", fileName);
        }

        file.close();

        EndianHelper::Endianness e = EndianHelper::IS_LITTLE_ENDIAN;
        //check if swap is necessary:
        if ((header.dim[0] < 0) || (header.dim[0] > 15)) {
            e = EndianHelper::IS_BIG_ENDIAN;
            header.swapEndianess();
        }

        if (header.sizeof_hdr != 348) {
            throw cgt::CorruptedFileException("Invalid header.sizeof_hdr", fileName);
        }

        if (!( (header.magic[0] == 'n') && (header.magic[2] == '1') && (header.magic[3] == 0) ))
            throw cgt::CorruptedFileException("Not a Nifti header!", fileName);

        if (header.magic[1] == '+') {
            if (!standalone)
                LWARNING("Tried to read standalone Nifti as hdr+img!");
            standalone = true;
        }
        else if (header.magic[1] == 'i') {
            if (!standalone)
                LWARNING("Tried to read hdr+img Nifti as standalone!");
            standalone = false;
        }
        else
            throw cgt::CorruptedFileException("Not a Nifti header!", fileName);

        cgt::ivec3 dimensions;
        dimensions.x = header.dim[1];
        dimensions.y = header.dim[2];
        dimensions.z = header.dim[3];

        size_t numVolumes = static_cast<size_t>(header.dim[4]);

        if (cgt::hor(cgt::lessThanEqual(dimensions, cgt::ivec3(0)))) {
            LERROR("Invalid resolution or resolution not specified: " << dimensions);
            throw cgt::CorruptedFileException("error while reading data", fileName);
        }

        cgt::vec3 spacing;
        spacing.x = header.pixdim[1];
        spacing.y = header.pixdim[2];
        spacing.z = header.pixdim[3];

//        int timeunit = XYZT_TO_TIME(header.xyzt_units);
        int spaceunit = XYZT_TO_SPACE(header.xyzt_units);

//         float dt = header.pixdim[4];
//         float toffset = header.toffset;
//         switch(timeunit) {
//             case NIFTI_UNITS_SEC:
//                 dt *= 1000.0f;
//                 toffset *= 1000.0f;
//                 break;
//             case NIFTI_UNITS_MSEC:
//                 //nothing to do
//                 break;
//             case NIFTI_UNITS_USEC:
//                 dt /= 1000.0f;
//                 toffset /= 1000.0f;
//                 break;
//         }

        switch(spaceunit) {
            case NIFTI_UNITS_MM:
                //nothing to do
                break;
            case NIFTI_UNITS_METER:
                spacing *= 1000.0f;
                break;
            case NIFTI_UNITS_MICRON:
                spacing /= 1000.0f;
                break;
            case NIFTI_UNITS_UNKNOWN:
            default:
                LWARNING("Unknown space unit!");
                break;
        }

        size_t numChannels = 1;
        WeaklyTypedPointer::BaseType baseType;

        switch(header.datatype) {
            case DT_UNSIGNED_CHAR:
                baseType = WeaklyTypedPointer::UINT8;
                break;
            case DT_SIGNED_SHORT:
                baseType = WeaklyTypedPointer::INT16;
                break;
            case DT_SIGNED_INT:
                baseType = WeaklyTypedPointer::INT32;
                break;
            case DT_FLOAT:
                baseType = WeaklyTypedPointer::FLOAT;
                break;
            case DT_RGB:
                baseType = WeaklyTypedPointer::UINT8;
                numChannels = 3;
                break;
            case DT_RGBA32:         /* 4 byte RGBA (32 bits/voxel)  */
                baseType = WeaklyTypedPointer::UINT8;
                numChannels = 4;
                break;
            case DT_INT8:           /* signed char (8 bits)         */
                baseType = WeaklyTypedPointer::INT8;
                break;
            case DT_UINT16:         /* unsigned short (16 bits)     */
                baseType = WeaklyTypedPointer::UINT16;
                break;
            case DT_UINT32:         /* unsigned int (32 bits)       */
                baseType = WeaklyTypedPointer::UINT32;
                break;
            case DT_INT64:          /* long long (64 bits)          */
            case DT_UINT64:         /* unsigned long long (64 bits) */
            case DT_FLOAT128:       /* long double (128 bits)       */
            case DT_COMPLEX128:     /* double pair (128 bits)       */
            case DT_COMPLEX256:     /* long double pair (256 bits)  */
            case DT_ALL:
            case DT_COMPLEX:
            case 0: //DT_NONE/DT_UNKNOWN
            case DT_BINARY:
            case DT_DOUBLE:
            default:
                throw cgt::UnsupportedFormatException("Unsupported datatype!");
        }

        size_t headerskip = static_cast<uint16_t>(header.vox_offset);

        std::string hdrFileName = fileName;
        if (!standalone)
            hdrFileName = cgt::FileSystem::fullBaseName(fileName) + ".hdr";


        cgt::mat4 pToW = cgt::mat4::identity;

        // Calculate transformation:
        if (header.sform_code > 0) {
            cgt::mat4 vToW(header.srow_x[0], header.srow_x[1], header.srow_x[2], header.srow_x[3],
                header.srow_y[0], header.srow_y[1], header.srow_y[2], header.srow_y[3],
                header.srow_z[0], header.srow_z[1], header.srow_z[2], header.srow_z[3],
                0.0f, 0.0f, 0.0f, 1.0f);

            cgt::mat4 wToV = cgt::mat4::identity;
            if(!vToW.invert(wToV)) {
                LERROR("Failed to invert voxel to world matrix!");
            }

            cgt::mat4 vToP = cgt::mat4::createScale(spacing); //no offset
            pToW = vToP * wToV;
        }
        else if (header.qform_code > 0) {
            float b = header.quatern_b;
            float c = header.quatern_c;
            float d = header.quatern_d;
            float a = static_cast<float>(sqrt(1.0-(b*b+c*c+d*d)));

            cgt::mat4 rot2(a*a+b*b-c*c-d*d,   2*b*c-2*a*d,       2*b*d+2*a*c,     0.0f,
                2*b*c+2*a*d,       a*a+c*c-b*b-d*d,   2*c*d-2*a*b,     0.0f,
                2*b*d-2*a*c,       2*c*d+2*a*b,       a*a+d*d-c*c-b*b, 0.0f,
                0.0f,              0.0f,              0.0f,            1.0f);

            float qfac = header.pixdim[0];
            if(fabs(qfac) < 0.1f)
                qfac = 1.0f;
            cgt::mat4 sc = cgt::mat4::createScale(cgt::vec3(1.0f, 1.0f, qfac));

            cgt::mat4 os = cgt::mat4::createTranslation(cgt::vec3(header.qoffset_x, header.qoffset_y, header.qoffset_z));
            pToW = os * rot2 * sc;
        }

        // Nifti transformations give us the center of the first voxel, we translate to correct:
        pToW = pToW * cgt::mat4::createTranslation(-spacing * 0.5f);
        // TODO: implement arbitrary transformations into ImageMappingInformation

        cgt::svec3 imageSize(dimensions);
        if (numVolumes <= 1) {
            ImageData* image = new ImageData(3, imageSize, numChannels);
            ImageRepresentationDisk::create(image, hdrFileName, baseType, headerskip, e);
            // Nifti transformations give us the center of the first voxel, we translate to correct:
            image->setMappingInformation(ImageMappingInformation(imageSize, cgt::vec3(-.5f) + p_imageOffset.getValue(), spacing * p_voxelSize.getValue(), pToW));
            dataContainer.addData(p_targetImageID.getValue(), image);
        }
        else {
            ImageSeries* is = new ImageSeries();
            size_t numBytes = hmul(imageSize) * (header.bitpix / 8);
            for (size_t i = 0; i < numVolumes; ++i) {
                ImageData* image = new ImageData(3, imageSize, numChannels);
                ImageRepresentationDisk::create(image, hdrFileName, baseType, headerskip + i * numBytes, e);
                // Nifti transformations give us the center of the first voxel, we translate to correct:
                image->setMappingInformation(ImageMappingInformation(imageSize, cgt::vec3(-.5f) + p_imageOffset.getValue(), spacing * p_voxelSize.getValue(), pToW));
                is->addImage(image);
            }
            dataContainer.addData(p_targetImageID.getValue(), is);
        }

    }

}
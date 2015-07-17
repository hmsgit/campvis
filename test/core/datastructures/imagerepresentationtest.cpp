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

#include "gtest/gtest.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationdisk.h"
#include "core/datastructures/imagerepresentationlocal.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/tools/simplejobprocessor.h"

using namespace campvis;

/**
 * Test class for ImageData class.
 */
class ImageRepresentationTest : public testing::Test {
protected:
    ImageRepresentationTest() 
        : _image(nullptr)
        , _size(10, 10, 9)
        , _ushortRep(nullptr)
        , _shortRep(nullptr)
        , _uintRep(nullptr)
        , _floatRep(nullptr)
    {
        // create image and load from disk
        _image = new ImageData(3, _size, 1);
        ImageRepresentationDisk::create(_image, CAMPVIS_SOURCE_DIR "/test/sampledata/10_10_9_image.raw", WeaklyTypedPointer::UINT16);
    }

    ~ImageRepresentationTest() {
    }

    void performComparisonTest() {
        // check that all representations are present
        EXPECT_TRUE(_localRep != nullptr);
        EXPECT_TRUE(_glRep != nullptr);
        EXPECT_TRUE(_ushortRep != nullptr);

        // download texture
        const cgt::Texture* tex = _glRep->getTexture();
        GLubyte* buffer = tex->downloadTextureToBuffer(GL_RED, GL_UNSIGNED_SHORT);
        uint16_t* voxels = reinterpret_cast<uint16_t*>(buffer);

        // compare both representations voxel-wise
        for (size_t i = 0; i < _image->getNumElements(); ++i) {
            uint16_t localVoxel = _ushortRep->getElement(i);
            uint16_t glVoxel = voxels[i];

            EXPECT_EQ(localVoxel, glVoxel);
        }

        delete [] buffer;
    }

    void convertLocalGl() {
        _localRep = _image->getRepresentation<ImageRepresentationLocal>();
        _glRep = _image->getRepresentation<ImageRepresentationGL>();
        _ushortRep = _image->getRepresentation< GenericImageRepresentationLocal<uint16_t, 1> >(false);
    }

    void convertGlLocal() {
        _glRep = _image->getRepresentation<ImageRepresentationGL>();
        _localRep = _image->getRepresentation<ImageRepresentationLocal>();
        _ushortRep = _image->getRepresentation< GenericImageRepresentationLocal<uint16_t, 1> >(false);
    }

    void performBasetypeConversionTest() {
        _localRep = _image->getRepresentation<ImageRepresentationLocal>();

        // perform various basetype converting conversions:
        _ushortRep = _image->getRepresentation< GenericImageRepresentationLocal<uint16_t, 1> >();
        _shortRep = _image->getRepresentation< GenericImageRepresentationLocal<int16_t, 1> >();
        _uintRep = _image->getRepresentation< GenericImageRepresentationLocal<uint32_t, 1> >();
        _floatRep = _image->getRepresentation< GenericImageRepresentationLocal<float, 1> >();

        // compare both representations voxel-wise
        for (size_t i = 0; i < _image->getNumElements(); ++i) {
            float ushortVoxel = _ushortRep->getElementNormalized(i, 0);
            float shortVoxel = _shortRep->getElementNormalized(i, 0);
            float uintVoxel = _uintRep->getElementNormalized(i, 0);
            float floatVoxel = _floatRep->getElementNormalized(i, 0);
            
            const uint32_t missingBit = std::numeric_limits<uint16_t>::max() - std::numeric_limits<int16_t>::max();
            const float tolerance = 2.f / missingBit;

            EXPECT_NEAR(ushortVoxel, shortVoxel, tolerance);
            EXPECT_FLOAT_EQ(ushortVoxel, uintVoxel);
            EXPECT_FLOAT_EQ(ushortVoxel, floatVoxel);
        }
    }

protected:
    ImageData* _image;
    cgt::svec3 _size;

    const ImageRepresentationLocal* _localRep;
    const ImageRepresentationGL* _glRep;
    const GenericImageRepresentationLocal<uint16_t, 1>* _ushortRep;
    const GenericImageRepresentationLocal<int16_t, 1>* _shortRep;
    const GenericImageRepresentationLocal<uint32_t, 1>* _uintRep;
    const GenericImageRepresentationLocal<float, 1>* _floatRep;
};

/**
 * Tests conversion Disk -> Local -> GL.
 * First performs conversion and then compares the values in each voxel .
 */
TEST_F(ImageRepresentationTest, conversion_disk_local_gl_test) {
    convertLocalGl();
    performComparisonTest();
}

/**
* Tests conversion Disk -> GL -> Local.
* First performs conversion and then compares the values in each voxel .
 */
TEST_F(ImageRepresentationTest, conversion_disk_gl_local_test) {
    convertGlLocal();
    performComparisonTest();
}

/**
* Tests conversion between different basetypes Disk -> Local<ushort> -> Local<ubyte> -> Local<float>.
* First performs conversion and then compares the values in each voxel .
 */
TEST_F(ImageRepresentationTest, basetype_conversion_test) {
    performBasetypeConversionTest();
}

/**
 * Tests multiple concurrent conversions.
 * Tests that no redundant representations are created.
 */
TEST_F(ImageRepresentationTest, concurrent_conversion_test) {
    tbb::atomic<int> _counter;
    _counter = 0;

    // hopefully, these jobs are spawned fast enough to simulate concurrent conversions...
    const int numInstantiations = 128;
    for (int i = 0; i < numInstantiations; ++i)
        SimpleJobProc.enqueueJob([&] () { this->convertLocalGl(); ++_counter; });

    while (_counter != numInstantiations)
        std::this_thread::yield();

    EXPECT_EQ(this->_image->getNumRepresentations(), 3U);
}

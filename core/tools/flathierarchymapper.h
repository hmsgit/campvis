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

#ifndef FLATHIERARCHYMAPPER_H__
#define FLATHIERARCHYMAPPER_H__

#define DIV_CEIL(x,y) ((x) > 0) ? (1 + ((x) - 1)/(y)) : ((x) / (y))

#define FOR_EACH_SVEC3(INDEX, POS, SIZE) \
    for (cgt::svec3 (INDEX) = (POS); (INDEX).z < (SIZE).z; ++(INDEX).z) \
        for ((INDEX).y = (POS).y; (INDEX).y < (SIZE).y; ++(INDEX).y) \
            for ((INDEX).x = (POS).x; (INDEX).x < (SIZE).x; ++(INDEX).x)

#define FOR_EACH_IVEC3(INDEX, POS, SIZE) \
    for (cgt::ivec3 (INDEX) = (POS); (INDEX).z < (SIZE).z; ++(INDEX).z) \
        for ((INDEX).y = (POS).y; (INDEX).y < (SIZE).y; ++(INDEX).y) \
            for ((INDEX).x = (POS).x; (INDEX).x < (SIZE).x; ++(INDEX).x)

#include "cgt/cgt_math.h"
#include "cgt/opengljobprocessor.h"
#include "cgt/texture.h"
#include "cgt/textureunit.h"
#include "cgt/vector.h"

#include "core/coreapi.h"
#include "core/classification/abstracttransferfunction.h"
#include "core/datastructures/datahandle.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/tools/concurrenthistogram.h"
#include "core/tools/typetraits.h"

#include <tbb/parallel_for.h>
#include <tbb/tick_count.h>

#include <algorithm>
#include <queue>

namespace {
    /// RGB to CIE XYZ colorconversion matrix
    static const cgt::mat3 rgbToXyz(0.412453f, 0.357580f, 0.180423f,
                                    0.212671f, 0.715160f, 0.072169f,
                                    0.019334f, 0.119193f, 0.950227f);
    /// reference white point for Lab color space
    static const cgt::vec3 labRef(95.047f, 100.f, 108.883f);

    /// total number of LOD levels
    static const size_t numLevels = 5;
    /// sizes of each LOD level in terms of pixels per dimension
    static const size_t levelSizes[5] = { 16, 8, 4, 2, 1 };


    /**
     * This method is used by the tight bin packing algorithm during LOD selection.
     * Advances the current placement cursor \a voxel by one block with respect to the given level.
     * \param	voxel   Current block placement cursor, will be altered by this function.
     * \param	level   Current block placement level.
     */
    void advance(cgt::svec3& voxel, size_t level) {
        // if we are at the top-most level, everything is simple => simply advance by one block
        if (level == 0) {
            voxel.x += levelSizes[0];
        }
        // otherwise, we're filling a 2x2x2 block, check what we've achieved so far:
        else if (voxel.x % levelSizes[level - 1] == 0) {
            // we're at this level's first block in X direction => go one block to the right
            voxel.x += levelSizes[level];
        }
        else {
            if (voxel.y % levelSizes[level - 1] == 0) {
                // we're at this level's first block in Y direction => go one block to the left and one up
                voxel.x -= levelSizes[level];
                voxel.y += levelSizes[level];
            }
            else {
                if (voxel.z % levelSizes[level - 1] == 0) {
                    // we're at this level's first block in Z direction => go one block to the left and down and one to the back
                    voxel.x -= levelSizes[level];
                    voxel.y -= levelSizes[level];
                    voxel.z += levelSizes[level];
                }
                else {
                    // this is the complex case: the 2x2x2 block of this level is full
                    // thus, go one block to the left, down and front and then do the same for level N-1.
                    voxel -= levelSizes[level];

                    // For the sake of clearer code, this is not written entirely non-recusive
                    // (as I will claim down there), but it's tail-recursive and no ctors/dtors 
                    // are called.
                    // Thus, a smart compiler should be able to produce really fast code out of this.
                    advance(voxel, level - 1);
                }
            }
        }
    }

}

namespace campvis {

    class ImageData;

    class CAMPVIS_CORE_API AbstractFlatHierarchyMapper {
    public:
        AbstractFlatHierarchyMapper(const ImageData* originalVolume, bool computeGradients);
        virtual ~AbstractFlatHierarchyMapper();

        static AbstractFlatHierarchyMapper* create(const ImageData* originalVolume, bool computeGradients);

        /**
         * 
         * \note    Requires a valid acquired OpenGL context!
         */
        virtual void selectLod(AbstractTransferFunction* tf, const cgt::svec3& hierarchyNumBlocks) = 0;

        /**
         * Returns the texture storing the resulting flat block hierarchy.
         * \return	_flatHierarchyTexture
         */
        cgt::Texture* getFlatHierarchyTexture();

        /**
         * Returns the index texture storing the lookup information for accessing blocks within _flatHierarchyTexture.
         * \return	_indexTexture
         */
        cgt::Texture* getIndexTexture();

        /**
         * Returns the index texture storing the lookup information for accessing blocks within _flatHierarchyTexture.
         * \return	_indexTexture
         */
        cgt::Texture* getGradientTexture();

        DataHandle _indexDH;
        DataHandle _flatHierarchyDH;
        DataHandle _gradientDH;

    protected:
        const ImageData* _originalVolume;       ///< Original version of the volume
        cgt::Texture* _flatHierarchyTexture;    ///< Texture storing the resulting flat block hierarchy
        cgt::Texture* _indexTexture;            ///< Index texture storing the lookup information for accessing blocks within _flatHierarchyTexture

        bool _computeGradientTexture;
        cgt::Texture* _gradientTexture;

        static std::string loggerCat_;
    };


    // ================================================================================================


    /**
     * Helper class to construct and manage a flat block hierarchy for volumes.
     * 
     * Inspired by the series of papers around "Transfer Function Based Adaptive Decompression
     * for Volume Rendering of Large Medical Data Sets" by Ljung et al.
     * The current implementation is working completely in RAM, thus no real out-of-core mapper.
     */
    template<typename BASETYPE>
    class FlatHierarchyMapper : public AbstractFlatHierarchyMapper {
    public:
        /// Type of one single image element
        typedef BASETYPE ElementType;

    private:
        /// Element type for the priority queue used during LOD selection process representing a single block.
        struct PqElement { 
            size_t index;           ///< index of the represented block
            float significance;     ///< significance of this block with respect to its current level (color deviation/memory costs)
        };
        /// Comparison functor for objects of PqElement type.
        /// Comparison is performed based on their significance.
        struct PqElementComparer {
            bool operator() (const PqElement& lhs, const PqElement& rhs) const {
                return lhs.significance < rhs.significance;
            }
        };

        /// Element type of a single bin for the simplified histogram used for the LOD selection.
        struct HistogramBin {
            ElementType _min;   ///< Minimum data value of this histogram bin.
            ElementType _max;   ///< Maximum data value of this histogram bin.
            int _count;         ///< Number of elements in this bin.
        };
        /// Typedef for the simplified histograms used for the LOD selection.
        typedef std::vector<HistogramBin> SimplifiedHistogramType;

        /**
         * Represents a single block within an arbitrary level. 
         * It does not store the data, it only stores the necessary meta information to access 
         * the block's voxels.
         */
        struct Block {
            Block(ElementType* baseElement, cgt::col4* baseGradient, cgt::svec3& size) 
                : _baseElement(baseElement)
                , _baseGradient(baseGradient)
                , _size(size)
                , _numElements(cgt::hmul(size))
                , _minimumValue(ElementType(std::numeric_limits<BASETYPE>::max()))
                , _maximumValue(ElementType(std::numeric_limits<BASETYPE>::min()))
                , _averageValue(ElementType(BASETYPE(0)))
            {
            }

            ElementType& getElement(const cgt::svec3& position) {
                cgtAssert(position.x + (position.y * _size.x) + (position.z * _size.x * _size.y) < _numElements, "Element access out of bounds!");
                return *(_baseElement + (position.x + (position.y * _size.x) + (position.z * _size.x * _size.y)));
            }

            cgt::col4& getGradient(const cgt::svec3& position) {
                cgtAssert(position.x + (position.y * _size.x) + (position.z * _size.x * _size.y) < _numElements, "Element access out of bounds!");
                return *(_baseGradient + (position.x + (position.y * _size.x) + (position.z * _size.x * _size.y)));
            }

            ElementType* _baseElement;          ///< pointer to the base element (i.e. at (0,0,0)) of this block
            cgt::col4* _baseGradient;           ///< pointer to the base gradient element (i.e. at (0,0,0)) of this block
            cgt::svec3 _size;                   ///< number of voxels in each dimension
            size_t _numElements;                ///< total number of elements in _data (i.e. sizeof(_data))

            ElementType _minimumValue;          ///< Minimum voxel value within this voxel
            ElementType _maximumValue;          ///< Maximum voxel value within this voxel
            ElementType _averageValue;          ///< Average voxel value within this voxel
            SimplifiedHistogramType _histogram; ///< simplified histogram of the voxel value distribution within this block
        };


        /**
         * A Level represents a hierarchy level.
         * A level contains the blocks with the voxel data.
         */
        struct Level {
            Level (const cgt::svec3& size, size_t blockSize, bool storeGradients)
                : _size(size)
                , _numBlocks(cgt::hmul(_size))
                , _blockSize(blockSize)
            {
                // initialize storage area for raw voxel data with all zeros
                size_t numElementsPerBlock = blockSize*blockSize*blockSize;
                _rawData.resize(_numBlocks * numElementsPerBlock, ElementType(BASETYPE(0)));

                // if this level is to store gradients, initialize their storage area
                if (storeGradients)
                    _rawGradients.resize(_numBlocks * numElementsPerBlock, cgt::col4(0, 0, 0, 0));

                // instantiate and initialize Blocks
                _blocks.reserve(_numBlocks);
                ElementType* baseElementPointer = &_rawData.front();
                cgt::col4* baseGradientPointer = (storeGradients) ? &_rawGradients.front() : nullptr;
                FOR_EACH_SVEC3(i, cgt::svec3(0, 0, 0), _size) {
                    _blocks.push_back(Block(baseElementPointer, baseGradientPointer, cgt::svec3(_blockSize)));
                    baseElementPointer += numElementsPerBlock;
                    if (storeGradients)
                        baseGradientPointer += numElementsPerBlock;
                }

            }

            Block& getBlock(const cgt::svec3& position) {
                cgtAssert(position.x + (position.y * _size.x) + (position.z * _size.x * _size.y) < _numBlocks, "Block access out of bounds!");
                return _blocks[position.x + (position.y * _size.x) + (position.z * _size.x * _size.y)];
            }

            cgt::svec3 indexToBlock(size_t index) {
                size_t z = index / (_size.x * _size.y);
                size_t y = (index % (_size.x * _size.y)) / _size.x;
                size_t x = index % _size.x;
                return cgt::svec3(x, y, z);
            }

            cgt::svec3 _size;                   ///< Total number of elements in this level (i.e. in _blocks)
            size_t _numBlocks;                  ///< total number of blocks (i.e. sizeof(_blocks))
            size_t _blockSize;                  ///< Size of the block (number of elements in each direction)

            std::vector<Block> _blocks;             ///< Linearized array of the blocks containing the meta information on how to access the raw voxel data.
            std::vector<ElementType> _rawData;      ///< Linearized array of the blocks' raw voxel data.
            std::vector<cgt::col4> _rawGradients;   ///< Linearized array of the blocks' gradient data
        };

    public:
        FlatHierarchyMapper(const ImageData* originalVolume, bool computeGradients);
        virtual ~FlatHierarchyMapper();


        /**
         * 
         * \note    Requires a valid acquired OpenGL context!
         */
        virtual void selectLod(AbstractTransferFunction* tf, const cgt::svec3& hierarchyNumBlocks);

    private:
        /**
         * Creates and initializes the storage of the block hierarchy.
         * Creates the storage for the block hierarchy and fills it with the voxel data. Thereby
         * performs the average-downsampling.
         */
        void createBlockHierarchy();

        /**
         * Little helper function mimicking the standard TF lookup as it is done in the GLSL shader.
         * Not the most beautiful software design, but it works for now.
         * \param	tf      Transfer function to use.
         * \param	data    Color data of the TF (since it is not directly accessible from \a tf, unfortunately).
         * \param	value   Intensity value to look up.
         * \return  Color after applying the TF to \a value.
         */
        cgt::vec4 lookupTf(AbstractTransferFunction* tf, cgt::vec4* data, float value);

        /**
         * Converts the given RGB color to CIE L*a*b* space.
         * \param	rgb Incoming color to convert.
         * \return	\a rgb converted to CIE L*a*b* space.
         */
        cgt::vec3 rgb2Lab(cgt::vec3 rgb) const;

        Level* _levels[numLevels];              ///< The hierarchy of levels. 5 Levels in total; level 0 has 16^3, level 4 is 1^3 voxels per block

    };

    // ================================================================================================

    template<typename BASETYPE>
    FlatHierarchyMapper<BASETYPE>::FlatHierarchyMapper(const ImageData* originalVolume, bool computeGradients)
        : AbstractFlatHierarchyMapper(originalVolume, computeGradients)
    {


        createBlockHierarchy();

    }

    template<typename BASETYPE>
    FlatHierarchyMapper<BASETYPE>::~FlatHierarchyMapper() {
        for (size_t i = 0; i < 5; ++i)
            delete _levels[i];
    }

    template<typename BASETYPE>
    void FlatHierarchyMapper<BASETYPE>::selectLod(AbstractTransferFunction* tf, const cgt::svec3& hierarchyNumBlocks) {
        const cgt::svec3 targetTextureSize = hierarchyNumBlocks * levelSizes[0];
        const size_t memoryBudget = cgt::hmul(targetTextureSize) * sizeof(ElementType);

        // first, let's get the transfer function data in a really complicated manner... :D
        // (sorry, there is no easier way with the current CAMPVis TF API)
        cgt::TextureUnit tfUnit;
        tfUnit.activate();
        size_t tfSize;
        tfSize = cgt::hmul(tf->getTexture()->getDimensions());
        GLubyte* tfRawData = tf->getTexture()->downloadTextureToBuffer(GL_RGBA, GL_FLOAT);

        // I really hope this is safe and doesn't break any strict aliasing rules...
        cgt::vec4* tfData = reinterpret_cast<cgt::vec4*>(tfRawData);

        tbb::tick_count startTime;
        startTime = tbb::tick_count::now();

        // now walk through the blocks and determine their significance with respect to the current TF
        Level& firstLevel = *_levels[0];
        std::vector<float> blockSignificances(firstLevel._numBlocks, 0.f);
        tbb::parallel_for(tbb::blocked_range<size_t>(0, firstLevel._numBlocks), [&](const tbb::blocked_range<size_t>& range) {
            for (size_t i = range.begin(); i < range.end(); ++i) {
                const Block& thisBlock = firstLevel._blocks[i];
                const SimplifiedHistogramType& histogram = thisBlock._histogram;
                const cgt::vec4 approximationColor = lookupTf(tf, tfData, TypeNormalizer::normalizeToFloat<BASETYPE>(thisBlock._averageValue));
                const cgt::vec3 approximationLuv = rgb2Lab(approximationColor.xyz() * approximationColor.a);

                float significance = 0;

                // compute DeltaE for every entry in the simplified histogram
                for (size_t j = 0; j < histogram.size(); ++j) {
                    const cgt::vec4 originalColor = lookupTf(tf, tfData,
                        (TypeNormalizer::normalizeToFloat<BASETYPE>(histogram[j]._max) - TypeNormalizer::normalizeToFloat<BASETYPE>(histogram[j]._min)) / 2.f
                        + TypeNormalizer::normalizeToFloat<BASETYPE>(histogram[j]._min));
                    const cgt::vec3 originalLuv = rgb2Lab(originalColor.xyz() * originalColor.a);

                    significance += cgt::length(approximationLuv - originalLuv) * histogram[j]._count * (histogram[j]._max - histogram[j]._min);
                }

                // alternative: use uniform sampling over the simplified histogram
                //for (si)

                blockSignificances[i] = significance;
            }
        });
        
        // now start the optimization with our priority queue:
        float memoryCosts[5];
        memoryCosts[4] = float(sizeof(ElementType) * 1 * 1 * 1);
        memoryCosts[3] = float(sizeof(ElementType) * 2 * 2 * 2);
        memoryCosts[2] = float(sizeof(ElementType) * 4 * 4 * 4);
        memoryCosts[1] = float(sizeof(ElementType) * 8 * 8 * 8);
        memoryCosts[0] = float(sizeof(ElementType) * 16 * 16 * 16);

        std::vector<size_t> currentLevels(firstLevel._numBlocks, 4);
        std::priority_queue<PqElement, std::vector<PqElement>, PqElementComparer> pq;

        // populate the priority queue with all blocks at minimum level
        for (size_t i = 0; i < firstLevel._numBlocks; ++i) {
            if (blockSignificances[i] > 32) {
                PqElement pqe = { i, blockSignificances[i] / (memoryCosts[3] - memoryCosts[4]) };
                pq.push(pqe);
            }
        }

        int bytesLeft = int(memoryBudget) - int(currentLevels.size() * memoryCosts[4]);
        while (bytesLeft > 0 && !pq.empty()) {
            // get top element
            PqElement e = pq.top();
            pq.pop();
            size_t levelNow = currentLevels[e.index];
            int bytesNeeded = int(memoryCosts[levelNow - 1] - memoryCosts[levelNow]);


            // check whether this block still fits:
            if (bytesLeft >= bytesNeeded) {
                // update block level
                --(currentLevels[e.index]);

                // update memory budget
                bytesLeft -= bytesNeeded;

                // reinsert block into PQ, if not reached top level yet
                if (levelNow > 1) {
                    e.significance = blockSignificances[e.index] / (memoryCosts[levelNow - 1] - memoryCosts[levelNow]);
                    pq.push(e);
                }
            }
        }


        std::vector<size_t> blockStatistics(5, 0);
        for (size_t i = 0; i < currentLevels.size(); ++i)
            ++(blockStatistics[currentLevels[i]]);

        // now start packing the blocks into one texture
        typedef cgt::Vector4<uint16_t> IndexType;
        std::vector<IndexType> blockLookupData(firstLevel._numBlocks, IndexType(uint16_t(0)));

        // Start packing with the largest blocks:
        // 
        // This is a variant of the bin packing problem. Furtunately, we have a well defined bin 
        // and block size, thus the problem itself is not NP hard. Nevertheless the following code 
        // is quite complex in order to allow densely packing the hierarchy in a direct 
        // non-recursive fashion:
        // - Start with packing all of the largest blocks (16^3), then all of the second-largest (8^3), etc...
        // - When packing blocks of level N, arrange them in a 2x2x2 fashion to build a block of level N-1 size
        // - This is continued recursively until we have 2x2x2 of these level N-1 blocks building a level N-2 block, etc...
        // - To avoid recursion, we have the currentVoxel variable keeping track of where we currently are.
        // - The modulo operation allows to infer the block level we're currently packing.
        cgt::TextureUnit fhUnit, gradientUnit, indexUnit;
        indexUnit.activate();
        _indexTexture = new cgt::Texture(GL_TEXTURE_3D, _levels[0]->_size, GL_RGBA16UI, cgt::Texture::NEAREST);

        fhUnit.activate();
        // TODO: the instantiation of _flatHierarchyTexture here is just a temporary standin
        //       Later, it just needs to created once in ctor and here will just be the pixel update
        //       (same as with the index texture)
        _flatHierarchyTexture = new cgt::Texture(GL_TEXTURE_3D, targetTextureSize, TypeTraits<BASETYPE, 1>::glInternalFormat, cgt::Texture::LINEAR);
        _flatHierarchyTexture->bind();
        cgt::svec3 currentVoxel(0, 0, 0);

        for (size_t level = 0; level < numLevels; ++level) {
            for (size_t i = 0; i < currentLevels.size(); ++i) {
                if (currentLevels[i] == level) {
                    cgtAssert(cgt::hand(cgt::lessThan(currentVoxel, targetTextureSize)), "The target texture for our flat block hierarchy is full. Something went wrong... :(");

                    // pack block and upload into texture
                    glTexSubImage3D(
                        GL_TEXTURE_3D, 0, 
                        GLint(currentVoxel.x), GLint(currentVoxel.y), GLint(currentVoxel.z),
                        GLsizei(levelSizes[level]), GLsizei(levelSizes[level]), GLsizei(levelSizes[level]),
                        GL_RED, TypeTraits<BASETYPE, 1>::glDataType, _levels[level]->_blocks[i]._baseElement);

                    LGL_ERROR;

                    // store the packing information in the index structure
                    blockLookupData[i] = IndexType(uint16_t(currentVoxel.x), uint16_t(currentVoxel.y), uint16_t(currentVoxel.z), uint16_t(levelSizes[level]));

                    // mark as packed
                    currentLevels[i] = 1337;

                    // advance currentVoxel
                    advance(currentVoxel, level);

                    // the above scheme completely fills a linear array of level 0 blocks. However,
                    // we have to make sure to stay within texture bounds. Thus,
                    if (currentVoxel.x >= targetTextureSize.x) {
                        currentVoxel.x = 0;
                        currentVoxel.y += levelSizes[0];

                        if (currentVoxel.y >= targetTextureSize.y) {
                            currentVoxel.y = 0;
                            currentVoxel.z += levelSizes[0];
                        }
                    }
                }
            }
        }

        const int LAKSDJ[17] = { 0, 4, 3, 0, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 };

        if (_computeGradientTexture) {
            gradientUnit.activate();
            _gradientTexture = new cgt::Texture(GL_TEXTURE_3D, targetTextureSize, GL_RGBA8, cgt::Texture::LINEAR);

            for (size_t i = 0; i < blockLookupData.size(); ++i) {
                glTexSubImage3D(
                    GL_TEXTURE_3D, 0, 
                    GLint(blockLookupData[i].x), GLint(blockLookupData[i].y), GLint(blockLookupData[i].z),
                    GLsizei(blockLookupData[i].w), GLsizei(blockLookupData[i].w), GLsizei(blockLookupData[i].w),
                    GL_RGBA, GL_UNSIGNED_BYTE, _levels[LAKSDJ[blockLookupData[i].w]]->_blocks[i]._baseGradient);
            }
        }

        // upload the index texture to the GPU
        indexUnit.activate();
        _indexTexture->bind();
        _indexTexture->uploadTexture(reinterpret_cast<GLubyte*>(&blockLookupData.front()), GL_RGBA_INTEGER, GL_UNSIGNED_SHORT);

        tbb::tick_count endTime = tbb::tick_count::now();
        LINFO("Duration for LOD section: " << (endTime - startTime).seconds());

        const size_t kbUsed = DIV_CEIL(memoryBudget, 1024);
        const size_t kbOriginal = DIV_CEIL(cgt::hmul(_originalVolume->getSize()) * sizeof(ElementType), 1024);
        LINFO("Block optimization complete, reduced " << kbOriginal << "KB into " << kbUsed << " KB (" << 100.0 * double(kbUsed) / double(kbOriginal) << "%).");
        LINFO("Block usage statistics:");
        for (size_t i = 0; i < blockStatistics.size(); ++i)
            LINFO("Level " << i << " (" << levelSizes[i] << "): " << blockStatistics[i]);

        // putting the flat hierarchy texture into an ImageData/DataHandle just for convenience, so that it's easier to debug :)
        ImageData* id = new ImageData(3, targetTextureSize, 1);
        ImageRepresentationGL::create(id, _flatHierarchyTexture);
        _flatHierarchyDH = DataHandle(id);

        ImageData* idg = new ImageData(3, targetTextureSize, 4);
        ImageRepresentationGL::create(idg, _gradientTexture);
        _gradientDH = DataHandle(idg);

        ImageData* id2 = new ImageData(3, firstLevel._size, 4);
        ImageRepresentationGL::create(id2, _indexTexture);
        IndexType* foo = new IndexType[blockLookupData.size()];
        memcpy(foo, &blockLookupData.front(), blockLookupData.size() * sizeof(IndexType));
        GenericImageRepresentationLocal<uint16_t, 4>::create(id2, foo);
        _indexDH = DataHandle(id2);
    }

    template<typename BASETYPE>
    void FlatHierarchyMapper<BASETYPE>::createBlockHierarchy() {
        const auto& totalVolumeSize = _originalVolume->getSize();
        const GenericImageRepresentationLocal<BASETYPE, 1>* rep = _originalVolume->getRepresentation< GenericImageRepresentationLocal<BASETYPE, 1> >(false);
        cgtAssert(rep != nullptr, "The requested image representation is 0. This must not happen, something went terribly wrong!");

        // compute the number of blocks needed
        size_t blockSize = 16;
        cgt::svec3 numBlocks = totalVolumeSize / blockSize;

        // the above computation will always floor, thus we should check, whether we need an additional block
        // TODO: currently the potentially needed additional border blocks are also created
        //       with full _blockSize size, even though their actual amount of voxels is less!
        //       This should be fixed, as it will later probably lead to uninitialized values
        //       or wrongly calculated histograms...
        const auto coveredVoxels = numBlocks * blockSize;
        for (size_t i = 0; i < 3; ++i) {
            if (totalVolumeSize[i] > coveredVoxels[i])
                ++numBlocks[i];
        }

        // instantiate each level and allocate storage
        for (size_t level = 0; level < 5; ++level) {
            _levels[level] = new Level(numBlocks, blockSize, _computeGradientTexture);
            blockSize /= 2;
        }

        // initialize first level
        Level& firstLevel = *_levels[0];
        const Interval<float>& intensityRange = rep->getNormalizedIntensityRange();
        const float NARF = 255.f / intensityRange.size();
        FOR_EACH_SVEC3(voxelPosition, cgt::svec3(0, 0, 0), totalVolumeSize) {
            const cgt::svec3 indexBlock = voxelPosition / size_t(16);
            const cgt::svec3 indexVoxel = voxelPosition - (indexBlock * size_t(16));
            firstLevel.getBlock(indexBlock).getElement(indexVoxel) = rep->getElement(voxelPosition);

            if (_computeGradientTexture) {
                // for now just central difference gradients for simplicity
                const float dx = (voxelPosition.x > 0) ? rep->getElementNormalized(voxelPosition - cgt::svec3(1, 0, 0), 0) : rep->getElementNormalized(voxelPosition, 0);
                const float dy = (voxelPosition.y > 0) ? rep->getElementNormalized(voxelPosition - cgt::svec3(0, 1, 0), 0) : rep->getElementNormalized(voxelPosition, 0);
                const float dz = (voxelPosition.z > 0) ? rep->getElementNormalized(voxelPosition - cgt::svec3(0, 0, 1), 0) : rep->getElementNormalized(voxelPosition, 0);

                const float mdx = (voxelPosition.x < totalVolumeSize.x - 1) ? rep->getElementNormalized(voxelPosition + cgt::svec3(1, 0, 0), 0) : rep->getElementNormalized(voxelPosition, 0);
                const float mdy = (voxelPosition.y < totalVolumeSize.y - 1) ? rep->getElementNormalized(voxelPosition + cgt::svec3(0, 1, 0), 0) : rep->getElementNormalized(voxelPosition, 0);
                const float mdz = (voxelPosition.z < totalVolumeSize.z - 1) ? rep->getElementNormalized(voxelPosition + cgt::svec3(0, 0, 1), 0) : rep->getElementNormalized(voxelPosition, 0);

                const cgt::vec3 gradient = cgt::vec3(dx - mdx, dy - mdy, dz - mdz) * 0.5f;
                const float l = cgt::length(gradient);
                const cgt::col4 foo(cgt::col4((gradient * 127.f / l) + 128.f, uint8_t(cgt::clamp(l*NARF, 0.f, 255.f))));
                firstLevel.getBlock(indexBlock).getGradient(indexVoxel) = foo;
            }
        }
        LINFO("First level initialized.");

        // compute min/max/avg for each block in first level
        // In order to avoid artifacts, we also have to consider a 1 voxel border around this block.
        FOR_EACH_SVEC3(indexBlock, cgt::svec3(0, 0, 0), firstLevel._size) {
            Block& b = firstLevel.getBlock(indexBlock);
            double sum = 0.0;

            const cgt::ivec3 voxelStart = cgt::clamp(cgt::ivec3(indexBlock) * int(levelSizes[0]) - int(1), cgt::ivec3(0), cgt::ivec3(_originalVolume->getSize()));
            const cgt::ivec3 voxelEnd = cgt::clamp(cgt::ivec3(indexBlock) * int(levelSizes[0]) + int(levelSizes[0] + 1), cgt::ivec3(0), cgt::ivec3(_originalVolume->getSize()));
            FOR_EACH_IVEC3(voxel, voxelStart, voxelEnd) {
                ElementType e = rep->getElement(voxel);
                b._minimumValue = std::min(b._minimumValue, e);
                b._maximumValue = std::max(b._maximumValue, e);
                sum += double(e);
            }
            b._averageValue = ElementType(sum / b._numElements);
        }
        LINFO("Min/max/avg computed.");

        // perform average downsampling accross levels
        for (size_t level = 1; level < 5; ++level) {
            Level& inputLevel = *_levels[level - 1];
            Level& outputLevel = *_levels[level];

            // traverse all blocks
            FOR_EACH_SVEC3(indexBlock, cgt::svec3(0, 0, 0), outputLevel._size) {
                Block& inputBlock = inputLevel.getBlock(indexBlock);
                Block& outputBlock = outputLevel.getBlock(indexBlock);

                // traverse all voxels of the target block
                FOR_EACH_SVEC3(indexVoxel, cgt::svec3(0, 0, 0), cgt::svec3(outputLevel._blockSize)) {
                    // for each voxel perform an averaging of the 8 corresponding voxels in the inputBlock.
                    double sum = 0.0;
                        
                    sum += double(inputBlock.getElement((indexVoxel * size_t(2)) + cgt::svec3(0, 0, 0)));
                    sum += double(inputBlock.getElement((indexVoxel * size_t(2)) + cgt::svec3(1, 0, 0)));
                    sum += double(inputBlock.getElement((indexVoxel * size_t(2)) + cgt::svec3(0, 1, 0)));
                    sum += double(inputBlock.getElement((indexVoxel * size_t(2)) + cgt::svec3(1, 1, 0)));
                    sum += double(inputBlock.getElement((indexVoxel * size_t(2)) + cgt::svec3(0, 0, 1)));
                    sum += double(inputBlock.getElement((indexVoxel * size_t(2)) + cgt::svec3(1, 0, 1)));
                    sum += double(inputBlock.getElement((indexVoxel * size_t(2)) + cgt::svec3(0, 1, 1)));
                    sum += double(inputBlock.getElement((indexVoxel * size_t(2)) + cgt::svec3(1, 1, 1)));

                    outputBlock.getElement(indexVoxel) = ElementType(sum / 8.0);

                    // do the same for the gradients...
                    if (_computeGradientTexture) {
                        cgt::dvec4 sum(0.0);

                        sum += cgt::dvec4(inputBlock.getGradient((indexVoxel * size_t(2)) + cgt::svec3(0, 0, 0)));
                        sum += cgt::dvec4(inputBlock.getGradient((indexVoxel * size_t(2)) + cgt::svec3(1, 0, 0)));
                        sum += cgt::dvec4(inputBlock.getGradient((indexVoxel * size_t(2)) + cgt::svec3(0, 1, 0)));
                        sum += cgt::dvec4(inputBlock.getGradient((indexVoxel * size_t(2)) + cgt::svec3(1, 1, 0)));
                        sum += cgt::dvec4(inputBlock.getGradient((indexVoxel * size_t(2)) + cgt::svec3(0, 0, 1)));
                        sum += cgt::dvec4(inputBlock.getGradient((indexVoxel * size_t(2)) + cgt::svec3(1, 0, 1)));
                        sum += cgt::dvec4(inputBlock.getGradient((indexVoxel * size_t(2)) + cgt::svec3(0, 1, 1)));
                        sum += cgt::dvec4(inputBlock.getGradient((indexVoxel * size_t(2)) + cgt::svec3(1, 1, 1)));

                        outputBlock.getGradient(indexVoxel) = cgt::col4(sum / 8.0);
                    }
                }
            }
        }
        LINFO("Downsampling completed.");


        // compute simplified histogram for each L16 block
        tbb::parallel_for(tbb::blocked_range<size_t>(0, firstLevel._numBlocks), [&](const tbb::blocked_range<size_t>& range) {
            for (size_t indexBlock = range.begin(); indexBlock < range.end(); ++indexBlock) {
                Block& b = firstLevel._blocks[indexBlock];

                // first compute the real histogram
                size_t numBuckets = std::min(size_t(128), size_t(b._maximumValue - b._minimumValue + 1));
                const BASETYPE initialBucketSize = (b._maximumValue - b._minimumValue + 1) / BASETYPE(numBuckets);

                // initialize histogram
                SimplifiedHistogramType& histogram = b._histogram;
                histogram.reserve(numBuckets);
                for (ElementType i = 0; i < numBuckets; ++i) {
                    HistogramBin hb = { b._minimumValue + i*initialBucketSize, b._minimumValue + (i + 1)*initialBucketSize - 1, 0 };
                    histogram.push_back(hb);
                }
                histogram.back()._max = b._maximumValue;

                // fill histogram with samples
                // In order to avoid artifacts, we also have to consider a 1 voxel border around this block.
                const cgt::ivec3 voxelStart = cgt::clamp(cgt::ivec3(firstLevel.indexToBlock(indexBlock)) * int(levelSizes[0]) - int(1), cgt::ivec3(0), cgt::ivec3(_originalVolume->getSize()));
                const cgt::ivec3 voxelEnd = cgt::clamp(cgt::ivec3(firstLevel.indexToBlock(indexBlock)) * int(levelSizes[0]) + int(levelSizes[0] + 1), cgt::ivec3(0), cgt::ivec3(_originalVolume->getSize()));
                FOR_EACH_IVEC3(voxel, voxelStart, voxelEnd) {
                    ElementType e = rep->getElement(voxel);
                    cgtAssert(e >= b._minimumValue && e <= b._maximumValue, "Voxel intensity out of range. Something went wrong!");
                    size_t bucket = std::min(size_t(e - b._minimumValue) / size_t(initialBucketSize), numBuckets - 1);
                    ++(histogram[bucket]._count);
                }

                // now cluster as long as possible until we have only 12 buckets left
                while (histogram.size() > 32) {
                    // find minimum distance
                    size_t minDistancePairIndex = 0;
                    size_t minDistancePairDistance = std::abs(histogram[0]._count - histogram[1]._count);

                    for (size_t i = 1; i < histogram.size() - 1; ++i) {
                        size_t pairDistance = std::abs(histogram[i]._count - histogram[i + 1]._count);
                        if (pairDistance < minDistancePairDistance) {
                            minDistancePairIndex = i;
                            minDistancePairDistance = pairDistance;
                        }
                    }

                    // merge pair into one element
                    histogram[minDistancePairIndex]._max = histogram[minDistancePairIndex + 1]._max;
                    histogram[minDistancePairIndex]._count += histogram[minDistancePairIndex + 1]._count;
                    std::move(histogram.begin() + minDistancePairIndex + 2, histogram.end(), histogram.begin() + minDistancePairIndex + 1);
                    histogram.pop_back();
                }
            }
        });
        LINFO("Simplified histograms computed.");

        int a = 1;
    }

    template<typename BASETYPE>
    cgt::vec4 FlatHierarchyMapper<BASETYPE>::lookupTf(AbstractTransferFunction* tf, cgt::vec4* data, float intensity) {
        if (intensity < tf->getIntensityDomain().x)
            return cgt::vec4(0.f);
        else if (intensity > tf->getIntensityDomain().y)
            return cgt::vec4(0.f);
        else {
            float mapped = (intensity - tf->getIntensityDomain().x) / (tf->getIntensityDomain().y - tf->getIntensityDomain().x);
            float t = mapped - floor(mapped);
            size_t indexL = size_t(mapped * tf->getSize().x);
            size_t indexR = std::min(indexL + 1, tf->getSize().x);
            return (t * data[indexL]) + ((1-t) * data[indexR]);
        }
            
    }

    template<typename BASETYPE>
    cgt::vec3 FlatHierarchyMapper<BASETYPE>::rgb2Lab(cgt::vec3 rgb) const {
        if (rgb == cgt::vec3(0.f))
            return cgt::vec3(0.f);

        // convert to XYZ
        rgb = rgbToXyz * rgb;

        // gamma correction
        rgb /= labRef;
        for (size_t i = 0; i < 3; ++i) {
            if (rgb.elem[i] > 0.008856f)
                rgb.elem[i] = pow(rgb.elem[i], 1.f/3.f);
            else
                rgb.elem[i] = (7.787f * rgb.elem[i]) + (16.f / 116.f);
        }

        // final conversion
        return cgt::vec3((116.f * rgb.y) - 16.f, 500.f * (rgb.x - rgb.y), 200.f * (rgb.y - rgb.z));
    }

}


#endif // FLATHIERARCHYMAPPER_H__

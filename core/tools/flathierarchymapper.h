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

#define FOR_EACH_VOXEL(INDEX, POS, SIZE) \
    for (cgt::svec3 (INDEX) = (POS); (INDEX).z < (SIZE).z; ++(INDEX).z) \
        for ((INDEX).y = (POS).y; (INDEX).y < (SIZE).y; ++(INDEX).y) \
            for ((INDEX).x = (POS).x; (INDEX).x < (SIZE).x; ++(INDEX).x)

#include "cgt/cgt_math.h"
#include "cgt/opengljobprocessor.h"
#include "cgt/vector.h"

#include "core/coreapi.h"

#include "core/classification/abstracttransferfunction.h"

#include "core/tools/concurrenthistogram.h"
#include "core/tools/hierarchicalclustering.h"
#include "core/tools/typetraits.h"

#include <algorithm>
#include <queue>

namespace {
    static const cgt::mat3 rgbToXyz(0.412453f, 0.357580f, 0.180423f,
                                    0.212671f, 0.715160f, 0.072169f,
                                    0.019334f, 0.119193f, 0.950227f);
    static const cgt::vec3 labRef(95.047f, 100.f, 108.883f);
    static const cgt::vec3 whitePoint = rgbToXyz * cgt::vec3(1.f, 1.f, 1.f);
    static const float ref_U = (4.f * whitePoint.x) / (whitePoint.x + 15.f*whitePoint.y + 3.f*whitePoint.z);
    static const float ref_V = (4.f * whitePoint.y) / (whitePoint.x + 15.f*whitePoint.y + 3.f*whitePoint.z);
}

namespace campvis {

    class ImageData;

    /**
     * Helper class to construct and manage a flat block hierarchy for volumes.
     * 
     * Inspired by the series of papers around "Transfer Function Based Adaptive Decompression
     * for Volume Rendering of Large Medical Data Sets" by Ljung et al.
     * The current implementation is working completely in RAM, thus no real out-of-core mapper.
     */
    template<typename BASETYPE>
    class FlatHierarchyMapper {
    public:
        /// Type of one single image element
        typedef BASETYPE ElementType;
        /// Type of the histogram over voxel values
        typedef ConcurrentGenericHistogramND<BASETYPE, 1> HistogramType;

    private:

        struct HistogramElement {
            ElementType _min;
            ElementType _max;
            int _count;
        };
        typedef std::vector<HistogramElement> SimplifiedHistogramType;

        /**
         * Represents a single block within an arbitrary level. 
         * It does not store the data, it only stores the necessary meta information to access 
         * the block's voxels.
         */
        struct Block {
            Block(ElementType* baseElement, cgt::svec3& size) 
                : _baseElement(baseElement)
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

            ElementType* _baseElement;          ///< pointer to the base element (i.e. at (0,0,0)) of this block
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
            Level (const cgt::svec3& size, size_t blockSize)
                : _size(size)
                , _numBlocks(cgt::hmul(_size))
                , _blockSize(blockSize)
            {
                // initialize storage area for raw voxel data with all zeros
                size_t numElementsPerBlock = blockSize*blockSize*blockSize;
                _rawData.resize(_numBlocks * numElementsPerBlock, ElementType(BASETYPE(0)));

                // instantiate and initialize Blocks
                _blocks.reserve(_numBlocks);
                ElementType* baseElementPointer = &_rawData.front();
                FOR_EACH_VOXEL(i, cgt::svec3(0, 0, 0), _size) {
                    _blocks.push_back(Block(baseElementPointer, cgt::svec3(_blockSize)));
                    baseElementPointer += numElementsPerBlock;
                }

            }

            Block& getBlock(const cgt::svec3& position) {
                cgtAssert(position.x + (position.y * _size.x) + (position.z * _size.x * _size.y) < _numBlocks, "Block access out of bounds!");
                return _blocks[position.x + (position.y * _size.x) + (position.z * _size.x * _size.y)];
            }

            cgt::svec3 _size;                   ///< Total number of elements in this level (i.e. in _blocks)
            size_t _numBlocks;                  ///< total number of blocks (i.e. sizeof(_blocks))
            size_t _blockSize;                  ///< Size of the block (number of elements in each direction)

            std::vector<Block> _blocks;         ///< Linearized array of the blocks containing the meta information on how to access the raw voxel data.
            std::vector<ElementType> _rawData;  ///< Linearized array of the blocks' raw voxel data.
        };

    public:
        FlatHierarchyMapper(const ImageData* originalVolume);
        ~FlatHierarchyMapper();


        void selectLod(AbstractTransferFunction* tf, size_t memoryBudget);

    private:
        /**
         * Creates and initializes the storage of the block hierarchy.
         * Creates the storage for the block hierarchy and fills it with the voxel data. Thereby
         * performs the average-downsampling.
         */
        void createBlockHierarchy();

        cgt::vec4 lookupTf(AbstractTransferFunction* tf, cgt::vec4* data, float value);
        cgt::vec3 rgb2Lab(cgt::vec3 rgb) const;

        const ImageData* _originalVolume;       ///< Original version of the volume
        Level* _levels[5];                      ///< The hierarchy of levels. 5 Levels in total; level 0 has 16^3, level 4 is 1^3 voxels per block
    };

    // ================================================================================================

    template<typename BASETYPE>
    FlatHierarchyMapper<BASETYPE>::FlatHierarchyMapper(const ImageData* originalVolume) 
        : _originalVolume(originalVolume)
    {
        cgtAssert(originalVolume->getNumChannels() == 1, "FlatHierarchyMapper supports only single channel volumes!");

        //_levels = {nullptr, nullptr, nullptr, nullptr, nullptr};
        createBlockHierarchy();
    }

    template<typename BASETYPE>
    FlatHierarchyMapper<BASETYPE>::~FlatHierarchyMapper() {
        for (size_t i = 0; i < 5; ++i)
            delete _levels[i];
    }


    template<typename BASETYPE>
    void FlatHierarchyMapper<BASETYPE>::selectLod(AbstractTransferFunction* tf, size_t memoryBudget) {
        // first, let's get the transfer function data in a really complicated manner... :D
        // (sorry, there is no easier way with the current CAMPVis TF API)
        GLubyte* tfRawData = nullptr;
        cgt::vec4* tfData = nullptr;
        size_t tfSize;
        {
            cgt::OpenGLJobProcessor::ScopedSynchronousGlJobExecution glJob;
            tfRawData = tf->getTexture()->downloadTextureToBuffer(GL_RGBA, GL_FLOAT);
            tfSize = cgt::hmul(tf->getTexture()->getDimensions());

            // I really hope this is safe and doesn't break any strict aliasing rules...
            tfData = reinterpret_cast<cgt::vec4*>(tfRawData);
        }


        // now walk through the blocks and determine their significance with respect to the current TF
        Level& firstLevel = *_levels[0];
        std::vector<float> blockSignificances;
        blockSignificances.reserve(firstLevel._numBlocks);
        for (size_t i = 0; i < firstLevel._numBlocks; ++i) {
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

            blockSignificances.push_back(significance);
        }

        // now start the optimization with our priority queue:
        float memoryCosts[5];
        memoryCosts[4] = float(sizeof(ElementType) * 1 * 1 * 1);
        memoryCosts[3] = float(sizeof(ElementType) * 2 * 2 * 2);
        memoryCosts[2] = float(sizeof(ElementType) * 4 * 4 * 4);
        memoryCosts[1] = float(sizeof(ElementType) * 8 * 8 * 8);
        memoryCosts[0] = float(sizeof(ElementType) * 16 * 16 * 16);

        std::vector<size_t> currentLevels(firstLevel._numBlocks, 4);
        struct PqElement { size_t index; float costs; };
        struct PqElementComparer {
            bool operator() (const PqElement& lhs, const PqElement& rhs) const {
                return lhs.costs < rhs.costs;
            }
        };

        std::priority_queue<PqElement, std::vector<PqElement>, PqElementComparer> pq;

        // populate the priority queue with all blocks at minimum level
        for (size_t i = 0; i < firstLevel._numBlocks; ++i) {
            PqElement pqe = { i, blockSignificances[i] / (memoryCosts[3] - memoryCosts[4]) };
            pq.push(pqe);
        }

        int bytesLeft = int(memoryBudget);
        while (bytesLeft > 0) {
            // get top element
            PqElement e = pq.top();
            pq.pop();

            // update block level
            --(currentLevels[e.index]);
            size_t levelNow = currentLevels[e.index];

            // update memory budget
            bytesLeft += int(memoryCosts[levelNow + 1] - memoryCosts[levelNow]);

            // reinsert block into PQ, if not reached top level yet
            if (levelNow > 0) {
                e.costs = blockSignificances[e.index] / (memoryCosts[levelNow - 1] - memoryCosts[levelNow]);
                pq.push(e);
            }
        }
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

        // instantiate each leven and allocate storage
        for (size_t level = 0; level < 5; ++level) {
            _levels[level] = new Level(numBlocks, blockSize);
            blockSize /= 2;
        }

        // initialize first level
        Level& firstLevel = *_levels[0];
        FOR_EACH_VOXEL(voxelPosition, cgt::svec3(0, 0, 0), totalVolumeSize) {
            const cgt::svec3 indexBlock = voxelPosition / size_t(16);
            const cgt::svec3 indexVoxel = voxelPosition - (indexBlock * size_t(16));
            firstLevel.getBlock(indexBlock).getElement(indexVoxel) = rep->getElement(indexBlock * firstLevel._blockSize + indexVoxel);
        }

        // compute min/max/avg for each block in first level
        FOR_EACH_VOXEL(indexBlock, cgt::svec3(0, 0, 0), firstLevel._size) {
            Block& b = firstLevel.getBlock(indexBlock);
            double sum = 0.0;
            for (size_t i = 0; i < b._numElements; ++i) {
                b._minimumValue = std::min(b._minimumValue, b._baseElement[i]);
                b._maximumValue = std::max(b._maximumValue, b._baseElement[i]);
                sum += double(b._baseElement[i]);
            }
            b._averageValue = ElementType(sum / b._numElements);
        }

        // perform average downsampling accross levels
        for (size_t level = 1; level < 5; ++level) {
            Level& inputLevel = *_levels[level - 1];
            Level& outputLevel = *_levels[level];

            // traverse all blocks
            FOR_EACH_VOXEL(indexBlock, cgt::svec3(0, 0, 0), outputLevel._size) {
                Block& inputBlock = inputLevel.getBlock(indexBlock);
                Block& outputBlock = outputLevel.getBlock(indexBlock);

                // traverse all voxels of the target block
                FOR_EACH_VOXEL(indexVoxel, cgt::svec3(0, 0, 0), cgt::svec3(outputLevel._blockSize)) {
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
                }
            }
        }


        // compute simplified histogram for each L16 block
        FOR_EACH_VOXEL(indexBlock, cgt::svec3(0, 0, 0), firstLevel._size) {
            Block& b = firstLevel.getBlock(indexBlock);

            // first compute the real histogram
            size_t numBuckets = std::min(size_t(128), size_t(b._maximumValue - b._minimumValue + 1));
            const ElementType initialBucketSize = (b._maximumValue - b._minimumValue + 1) / ElementType(numBuckets);

            // initialize histogram
            SimplifiedHistogramType& histogram = b._histogram;
            histogram.reserve(numBuckets);
            for (ElementType i = 0; i < numBuckets; ++i) {
                HistogramElement he = { b._minimumValue + i*initialBucketSize, b._minimumValue + (i+1)*initialBucketSize - 1, 0 };
                histogram.push_back(he);
            }
            histogram.back()._max = b._maximumValue;

            // fill histogram with samples
            for (size_t i = 0; i < b._numElements; ++i) {
                ElementType e = b._baseElement[i];
                cgtAssert(e >= b._minimumValue && e <= b._maximumValue, "Voxel intensity out of range. Something went wrong!");
                size_t bucket = std::min(size_t(e - b._minimumValue) / initialBucketSize, numBuckets - 1);
                ++(histogram[bucket]._count);
            }

            // now cluster as long as possible until we have only 12 buckets left
            while (histogram.size() > 12) {
                // find minimum distance
                size_t minDistancePairIndex = 0;
                size_t minDistancePairDistance = std::abs(histogram[0]._count - histogram[1]._count);

                for (size_t i = 1; i < histogram.size() - 1; ++i) {
                    size_t pairDistance = std::abs(histogram[i]._count - histogram[i+1]._count);
                    if (pairDistance < minDistancePairDistance) {
                        minDistancePairIndex = i;
                        minDistancePairDistance = pairDistance;
                    }
                }

                // merge pair into one element
                histogram[minDistancePairIndex]._max = histogram[minDistancePairIndex+1]._max;
                histogram[minDistancePairIndex]._count += histogram[minDistancePairIndex+1]._count;
                std::move(histogram.begin()+minDistancePairIndex+2, histogram.end(), histogram.begin()+minDistancePairIndex+1);
                histogram.pop_back();
            }

            int bcccc = 1;
        }

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

        //const cgt::vec3 xyz = rgbToXyz * rgb;
        //
        //if (xyz.y/whitePoint.y > 0.008856f)
        //    rgb.x = 116.f * (std::pow(xyz.y/whitePoint.y, 1.f/3.f)) - 16.f;
        //else
        //    rgb.x = 903.3f * xyz.y/whitePoint.y;
        //
        //rgb.y = (4.f * xyz.x) / (xyz.x + 15.f*xyz.y + 3.f*xyz.z);
        //rgb.z = (9.f * xyz.y) / (xyz.x + 15.f*xyz.y + 3.f*xyz.z);
        //
        //rgb.y = 13.f * rgb.x * (rgb.y - ref_U);
        //rgb.z = 13.f * rgb.x * (rgb.z - ref_V);
        //return rgb;
    }

}


#endif // FLATHIERARCHYMAPPER_H__

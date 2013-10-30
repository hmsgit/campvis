#ifndef VOLUMEBRICKING_H__
#define VOLUMEBRICKING_H__

#include "tgt/vector.h"

#include "core/datastructures/imagedata.h"

#include <vector>

namespace tgt {
    class Texture;
}

namespace campvis {

    class BinaryBrickedVolume {
    public:
        BinaryBrickedVolume(const ImageData* referenceImage, size_t brickSize);

        ~BinaryBrickedVolume();


        /**
         * Gets the number of bricks in each dimension.
         * \return  _dimBricks
         */
        const tgt::svec3& getNumBricks() const;

        /**
         * Gets the number of brick indices (= hmul(_dimBricks)).
         * \return  _numBrickIndices
         */
        size_t getNumBrickIndices() const;

        size_t getBrickSize() const;


        /**
         * Returns the boolean value for the brick with index \a brickIndex.
         * \param   brickIndex  Lookup brick index
         * \return  The boolean value for the brick with index \a brickIndex.
         */
        bool getValueForIndex(size_t brickIndex) const;

        /**
         * Sets the boolean value for the brick with index \a brickIndex to \a value.
         * \param   brickIndex  Lookup brick index
         * \param   value       The new boolean value for this brick.
         */
        void setValueForIndex(size_t brickIndex, bool value);

        /**
         * Return a vector of all voxels positions in the reference image that are in the brick with the index \a brickIndex.
         * \param   brickIndex  Lookup brick index.
         * \return  A vector of all voxels positions in the reference image that are in the brick with the index \a brickIndex.
         */
        std::vector<tgt::svec3> getAllVoxelsForBrick(size_t brickIndex) const;


        tgt::Texture* exportToImageData() const;

    private:
        /**
         * Returns the brick coordinates for the brick with index \a brickIndex.
         * \param   brickIndex  The Brick index to look up
         * \return  The corresponding 3D brick coordinates.
         */
        tgt::svec3 indexToBrick(size_t brickIndex) const;

        /**
         * Transforms brick coordinates to the corresponding index.
         * \param   brick   Brick coordinates.
         * \return  The corresponding index in if all bricks are in contiguous storage.
         */
        size_t brickToIndex(const tgt::svec3& brick) const;



        const ImageData* _referenceImage;       ///< the reference image
        size_t _brickSize;                      ///< number of voxels a brick is covering in each dimension

        tgt::svec3 _dimBricks;                  ///< number of bricks in each dimension
        size_t _numBrickIndices;                ///< number of brick indices (= hmul(_dimPackedBricks))

        uint8_t* _bricks;                       ///< the densly packed bricks
        tgt::svec3 _dimPackedBricks;            ///< number of elements when bricks are tightly packed
        size_t _numElementsInBricksArray;       ///< number of elements in _bricks
       
    };
}

#endif // VOLUMEBRICKING_H__

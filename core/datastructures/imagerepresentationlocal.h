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

#ifndef IMAGEREPRESENTATIONLOCAL_H__
#define IMAGEREPRESENTATIONLOCAL_H__

#include "core/datastructures/abstractimagerepresentation.h"

#include "core/tools/concurrenthistogram.h"
#include "core/tools/endianhelper.h"
#include "core/tools/interval.h"
#include "core/tools/weaklytypedpointer.h"

namespace campvis {

    /**
     * Abstract base class for storing image data in the local memory.
     * 
     * \todo    implement padding, add some kind of cool iterators
     */
    class ImageRepresentationLocal : public AbstractImageRepresentation {
    public:
        typedef ConcurrentGenericHistogramND<float, 1> IntensityHistogramType;

        /**
         * Creates a new ImageData representation in local memory.
         *
         * \param   dimensionality  Dimensionality of data
         * \param   size            Size of this image (number of elements per dimension)
         * \param   baseType        Base type of the image data.
         * \param   numChannels     Number of channels per image element.
         */
        ImageRepresentationLocal(const ImageData* parent, WeaklyTypedPointer::BaseType baseType, size_t numChannels);

        /**
         * Destructor
         */
        virtual ~ImageRepresentationLocal();

        /// \see AbstractData::clone()
        virtual ImageRepresentationLocal* clone() const = 0;

        /// \see ImageData::getSubImage()
        virtual ImageRepresentationLocal* getSubImage(const tgt::svec3& llf, const tgt::svec3& urb) const = 0;

        /**
         * Returns a WeaklyTypedPointer to the image data.
         * \note    The pointer is still owned by this ImageRepresentationLocal. If you want a copy, use clone().
         * \return  A WeaklyTypedPointer to the image data.
         */
        virtual const WeaklyTypedPointer getWeaklyTypedPointer() const = 0;
        
        /**
         * Returns the normalized value of the element at the given index and channel.
         *  - for \em unsigned integer types, the value range is mapped linearly to [0.0;1.0]
         *  - for \em signed integer types, the value range is mapped linearly to [-1.0;1.0]
         *  - floating point types are not mapped
         * 
         * Simple algorithms on images might not always want to test for the actual base data type.
         * For them access to the normalized element values provided here might be enough.
         * 
         * \note    This method is virtual => know the costs!
         * \sa      ImageRepresentationLocal::setElementNormalized
         * \param   index       Element index
         * \param   channel     Image channel
         * \return  A normalized float representation of the element at the given position and channel.
         */
        virtual float getElementNormalized(size_t index, size_t channel) const = 0;

        /**
         * Returns the normalized value of the element at the given position and channel.
         *  - for \em unsigned integer types, the value range is mapped linearly to [0.0;1.0]
         *  - for \em signed integer types, the value range is mapped linearly to [-1.0;1.0]
         *  - floating point types are not mapped
         * 
         * Simple algorithms on images might not always want to test for the actual base data type.
         * For them access to the normalized element values provided here might be enough.
         * 
         * \note    This method is more expensive than the version directly using the element index.
         * \note    This method is virtual => know the costs!
         * \sa      ImageRepresentationLocal::setElementNormalized
         * \param   position    Element position within the image
         * \param   channel     Image channel
         * \return  A normalized float representation of the element at the given position and channel.
         */
        virtual float getElementNormalized(const tgt::svec3& position, size_t channel) const = 0;

        /**
         * Returns the normalized value of the element at the given position and channel using linear interpolation.
         *  - for \em unsigned integer types, the value range is mapped linearly to [0.0;1.0]
         *  - for \em signed integer types, the value range is mapped linearly to [-1.0;1.0]
         *  - floating point types are not mapped
         * 
         * Simple algorithms on images might not always want to test for the actual base data type.
         * For them access to the normalized element values provided here might be enough.
         * 
         * \note    This method is virtual => know the costs!
         * \sa      ImageRepresentationLocal::getElementNormalized
         * \param   position    Element position within the image
         * \param   channel     Image channel
         * \return  A normalized float representation of the element at the given position and channel.
         */
        virtual float getElementNormalizedLinear(const tgt::vec3& position, size_t channel) const = 0;

        /**
         * Sets the element at the given index and channel denormalized from the given value \a value.
         *  - for \em unsigned integer types, the value range is mapped linearly to [0.0;1.0]
         *  - for \em signed integer types, the value range is mapped linearly to [-1.0;1.0]
         *  - floating point types are not mapped
         * 
         * Simple algorithms on images might not always want to test for the actual base data type.
         * For them access to the normalized element values provided here might be enough.
         * 
         * \note    This method is virtual => know the costs!
         * \sa      ImageRepresentationLocal::getElementNormalized
         * \param   index       Element index
         * \param   channel     Image channel
         * \param   value       Normalized Value to set
         */
        virtual void setElementNormalized(size_t index, size_t channel, float value) = 0;

        /**
         * Sets the element at the given position and channel denormalized from the given value \a value.
         *  - for \em unsigned integer types, the value range is mapped linearly to [0.0;1.0]
         *  - for \em signed integer types, the value range is mapped linearly to [-1.0;1.0]
         *  - floating point types are not mapped
         * 
         * Simple algorithms on images might not always want to test for the actual base data type.
         * For them access to the normalized element values provided here might be enough.
         * 
         * \note    This method is more expensive than the version directly using the element index.
         * \note    This method is virtual => know the costs!
         * \sa      ImageRepresentationLocal::getElementNormalized
         * \param   position    Element position within the image
         * \param   channel     Image channel
         * \param   value       Normalized Value to set
         */
        virtual void setElementNormalized(const tgt::svec3& position, size_t channel, float value) = 0;

        /**
         * Returns the range of normalized intensities.
         * All
         * \sa      getElementNormalized()
         * \note    The intensity range is computed using lazy evaluation.
         * \return  _normalizedIntensityRange
         */
        const Interval<float>& getNormalizedIntensityRange() const;

        /**
         * Returns the intensity distribution normalized to float as 1D histogram.
         * \note    The intensity histogram is computed using lazy evaluation, hence, computation
         *          may take some time.
         * \return  _intensityHistogram
         */
        const IntensityHistogramType& getIntensityHistogram() const;

    protected:
        /**
         * Computes the normalized intensity range.
         */
        void computeNormalizedIntensityRange() const;

        /**
         * Computes the intensity histogram.
         */
        void computeIntensityHistogram() const;

        WeaklyTypedPointer::BaseType _baseType;     ///< Base type of the image data
        size_t _numChannels;                        ///< Number of channels per image element.

        mutable tbb::atomic<bool> _intensityRangeDirty;         ///< Flag whether _normalizedIntensityRange is dirty and has to be recomputed
        mutable Interval<float> _normalizedIntensityRange;      ///< Range of the normalized intensities, mutable to allow lazy instantiation
        mutable IntensityHistogramType* _intensityHistogram;    ///< Intensity histogram, mutable to allow lazy instantiation

        static const std::string loggerCat_;

    private:
        // We don't want this data to be copied - clone() must be enough
        // (read: We are too lazy to implement a correct copy constructor / assignment-operator)
         ImageRepresentationLocal(const ImageRepresentationLocal& rhs);
         ImageRepresentationLocal& operator=(const ImageRepresentationLocal& rhs);
    };

}

#endif // IMAGEREPRESENTATIONLOCAL_H__

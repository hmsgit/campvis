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

#ifndef GENERICIMAGEREPRESENTATIONLOCAL_H__
#define GENERICIMAGEREPRESENTATIONLOCAL_H__

#include "core/datastructures/imagerepresentationlocal.h"
#include "core/tools/typetraits.h"

#include <cstring>  // needed for memcpy

namespace campvis {

    /**
     * Templated version of ImageRepresentationLocal, storing image data in the local memory.
     *
     * \sa      TypeTraits
     * \tparam  BASETYPE    Base type of the image data (type of a single channel of an image element)
     * \tparam  NUMCHANNELS Number of channels of the image data.
     */
    template<typename BASETYPE, size_t NUMCHANNELS>
    class GenericImageRepresentationLocal : public ImageRepresentationLocal {
    public:
        /// Type of one single image element
        typedef typename TypeTraits<BASETYPE, NUMCHANNELS>::ElementType ElementType;
        /// Type of this template instantiation
        typedef GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS> ThisType;

        /**
         * Template specialization of the ScopedRepresentation defined in GenericAbstractImageRepresentation<T>
         * since some compiler get confused by the nested templates.
         * \note    This is a simple copy and paste from GenericAbstractImageRepresentation<T> with
         *          explicitly named template parameter.
         */
        struct ScopedRepresentation {
            /**
             * Creates a new DataHandle to the data item with the key \a name in \a dc, that behaves like a const GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>*.
             * \param   dc      DataContainer to grab data from
             * \param   name    Key of the DataHandle to search for
             * \param   silent  Flag whether debug messages when no matching data is found should be silenced (defaults to false).
             */
            ScopedRepresentation(const DataContainer& dc, const std::string& name, bool silent = false)
                : dh(dc.getData(name))
                , data(0)
                , representation(0) 
            {
                if (dh.getData() != 0) {
                    data = dynamic_cast<const ImageData*>(dh.getData());
                    if (data != 0) {
                        representation = data->getRepresentation< GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS> >();
                    }
                    else {
                        if (!silent)
                            LDEBUGC("CAMPVis.core.ScopedTypedData", "Found DataHandle with id '" << name << "', but it is of wrong type (" << typeid(*dh.getData()).name() << " instead of " << typeid(GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>).name() << ").");
                    }

                    if (data == 0 || representation == 0) {
                        dh = DataHandle(0);
                    }
                }
                else {
                    if (!silent)
                        LDEBUGC("CAMPVis.core.ScopedRepresentation", "Could not find a DataHandle with id '" << name << "' in DataContainer '" << dc.getName() << "'.");
                }
            };
            
            /**
             * Creates a new ScopedRepresentation for the given DataHandle, that behaves like a T*.
             * \param   dh  Source DataHandle
             */
            ScopedRepresentation(DataHandle dataHandle)
                : dh(dataHandle)
                , data(0)
                , representation(0) 
            {
                if (dh.getData() != 0) {
                    data = dynamic_cast<const ImageData*>(dh.getData());
                    if (data != 0) {
                        representation = data->getRepresentation<ThisType>();
                    }
                    if (data == 0 || representation == 0) {
                        dh = DataHandle(0);
                    }
                }
            };

            /**
             * Implicit conversion operator to const GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>*.
             * \return  The image representation of the image in the DataHandle, may be 0 when no DataHandle was found,
             *           the data is of the wrong type, or no suitable representation was found.
             */
            operator const GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>*() {
                return representation;
            }

            /**
             * Implicit arrow operator to const GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>*.
             * \return  The image representation of the image in the DataHandle, may be 0 when no DataHandle was found,
             *           the data is of the wrong type, or no suitable representation was found.
             */
            const GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* operator->() const {
                return representation;
            }

            /**
             * Gets the DataHandle.
             * \return dh
             */
            DataHandle getDataHandle() const {
                return dh;
            }

            /**
             * Returns the parent ImageData of this image representation.
             * \return data
             */
            const ImageData* getImageData() const {
                return data;
            }

        private:
            /// Not copy-constructable
            ScopedRepresentation(const ScopedRepresentation& rhs);
            /// Not assignable
            ScopedRepresentation& operator=(const ScopedRepresentation& rhs);

            DataHandle dh;                  ///< DataHandle
            const ImageData* data;          ///< strongly-typed pointer to data, may be 0
            const GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* representation;        ///< strongly-typed pointer to the image representation, may be 0
        };

        /**
         * Creates a new GenericImageRepresentationLocal with the given parameters and automatically
         * adds it to \a parent which will take ownerwhip.
         *
         * \note    You do \b not own the returned pointer.
         *
         * \param   parent     Image this representation represents, must not be 0, will take ownership of the returned pointer.
         * \param   data    Pointer to the image data, must not be 0, GenericImageRepresentationLocal takes ownership of this pointer!
         * \return  A pointer to the newly created ImageRepresentationDisk, you do \b not own this pointer!
         */
        static GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* create(const ImageData* parent, ElementType* data);

        /**
         * Destructor
         */
        virtual ~GenericImageRepresentationLocal();


        /// \see AbstractImageRepresentation::clone()
        virtual ThisType* clone(ImageData* newParent) const;

        /// \see AbstractImageRepresentation::getLocalMemoryFootprint()
        virtual size_t getLocalMemoryFootprint() const;

        /// \see AbstractImageRepresentation::getVideoMemoryFootprint()
        virtual size_t getVideoMemoryFootprint() const;

        /**
         * Returns a WeaklyTypedPointer to the image data.
         * \note    The pointer is still owned by this ImageRepresentationLocal. If you want a copy, use clone().
         * \return  A WeaklyTypedPointer to the image data.
         */
        virtual const WeaklyTypedPointer getWeaklyTypedPointer() const;

        /// \see ImageRepresentationLocal::getElementNormalized
        virtual float getElementNormalized(size_t index, size_t channel) const;

        /// \see ImageRepresentationLocal::getElementNormalized
        virtual float getElementNormalized(const cgt::svec3& position, size_t channel) const;

        /// \see ImageRepresentationLocal::getElementNormalizedLinear
        virtual float getElementNormalizedLinear(const cgt::vec3& position, size_t channel) const;

        /// \see ImageRepresentationLocal::setElementNormalized
        virtual void setElementNormalized(size_t index, size_t channel, float value);

        /// \see ImageRepresentationLocal::setElementNormalized
        virtual void setElementNormalized(const cgt::svec3& position, size_t channel, float value);


        /**
         * Returns the image element at the given index \a index.
         * \param   index   Array index of the image element to return.
         * \return  Image element at index \a index.
         */
        ElementType& getElement(size_t index);

        /**
         * Returns the image element at the given position in image space.
         * \param   position    Pixel/voxel coordinates of the image element to return.
         * \return  Image element at the coordinates \a position.
         */
        ElementType& getElement(const cgt::svec3& position);

        /**
         * Returns the image element at the given index \a index.
         * \param   index   Array index of the image element to return.
         * \return  Image element at index \a index.
         */
        const ElementType& getElement(size_t index) const;

        /**
         * Returns the image element at the given position in image space.
         * \param   position    Pixel/voxel coordinates of the image element to return.
         * \return  Image element at the coordinates \a position.
         */
        const ElementType& getElement(const cgt::svec3& position) const;

        /**
         * Sets the image element at the given index to the value \a value.
         * \param   index   Array index of the image element to change.
         * \param   value   New value of the specified image element.
         */
        void setElement(size_t index, const ElementType& value);

        /**
         * Sets the image element at the given position in image space to the value \a value.
         * \param   position    Pixel/voxel coordinates of the image element to change.
         * \param   value       New value of the specified image element.
         */
        void setElement(const cgt::svec3& position, const ElementType& value);

        /**
         * Returns a pointer to the image data.
         * \return  _data
         */
        ElementType* getImageData();

        /**
         * Returns a const pointer to the image data.
         * \return  _data
         */
        const ElementType* getImageData() const;


        /**
         * Returns the image element at the given coordinates \a position using bi-/trilinear filtering.
         * \param   position    Pixel/voxel coordinates of the image element to return.
         * \return  Bi-/Trilinear filtered image element at the specified coordinates.
         */
        ElementType getElementLinear(const cgt::vec3 position) const;

    protected:
        /**
         * Creates a new strongly typed ImageData object storing the image in the local memory.
         * 
         * \param   parent  Image this representation represents, must not be 0.
         * \param   data    Pointer to the image data, must not be 0, GenericImageRepresentationLocal takes ownership of this pointer!
         */
        GenericImageRepresentationLocal(ImageData* parent, ElementType* data);

        ElementType* _data;

        static const std::string loggerCat_;

    };

// = Template implementation ======================================================================

    template<typename BASETYPE, size_t NUMCHANNELS>
    const std::string campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::loggerCat_ = "CAMPVis.core.datastructures.GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>";
    
    template<typename BASETYPE, size_t NUMCHANNELS>
    campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::create(const ImageData* parent, ElementType* data) {
        ThisType* toReturn = new ThisType(const_cast<ImageData*>(parent), data);
        toReturn->addToParent();
        return toReturn;
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::GenericImageRepresentationLocal(ImageData* parent, ElementType* data)
        : ImageRepresentationLocal(parent, TypeTraits<BASETYPE, NUMCHANNELS>::weaklyTypedPointerBaseType)
        , _data(data)
    {
        cgtAssert(_parent->getNumChannels() == NUMCHANNELS, "Number of channels must match parent image's number of channels!");
        if (_data == 0) {
            size_t numElements = getNumElements();
            _data = new ElementType[numElements];
            memset(_data, 0, numElements * TypeTraits<BASETYPE, NUMCHANNELS>::elementSize);
        }
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::~GenericImageRepresentationLocal() {
        delete [] _data;
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::clone(ImageData* newParent) const {
        size_t numElements = getNumElements();
        ElementType* newData = new ElementType[numElements];
        memcpy(newData, _data, numElements * sizeof(ElementType));

        return ThisType::create(newParent, newData);
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    size_t campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getLocalMemoryFootprint() const {
        return sizeof(*this) + getNumElements() * sizeof(ElementType);
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    size_t campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getVideoMemoryFootprint() const {
        return 0;
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    const WeaklyTypedPointer campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getWeaklyTypedPointer() const {
        return WeaklyTypedPointer(TypeTraits<BASETYPE, NUMCHANNELS>::weaklyTypedPointerBaseType, NUMCHANNELS, _data);
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    float campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getElementNormalized(size_t index, size_t channel) const {
        cgtAssert(channel >= 0 && channel < NUMCHANNELS, "Channel out of bounds!");
        return TypeNormalizer::normalizeToFloat(TypeTraits<BASETYPE, NUMCHANNELS>::getChannel(getElement(index), channel));
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    float campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getElementNormalized(const cgt::svec3& position, size_t channel) const {
        return getElementNormalized(_parent->positionToIndex(position), channel);
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    void campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::setElementNormalized(size_t index, size_t channel, float value) {
        cgtAssert(channel >= 0 && channel < NUMCHANNELS, "Channel out of bounds!");
        TypeTraits<BASETYPE, NUMCHANNELS>::setChannel(getElement(index), channel, TypeNormalizer::denormalizeFromFloat<BASETYPE>(value));
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    void campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::setElementNormalized(const cgt::svec3& position, size_t channel, float value) {
        setElementNormalized(_parent->positionToIndex(position), channel, value);
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    typename campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::ElementType& campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getElement(size_t position) {
        cgtAssert(position >= 0 && position < getNumElements(), "Position out of bounds!");
        return _data[position];
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    typename campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::ElementType& campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getElement(const cgt::svec3& position) {
        return getElement(_parent->positionToIndex(position));
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    const typename campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::ElementType& campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getElement(size_t position) const {
        cgtAssert(position >= 0 && position < getNumElements(), "Position out of bounds!");
        return _data[position];
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    const typename campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::ElementType& campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getElement(const cgt::svec3& position) const {
        return getElement(_parent->positionToIndex(position));
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    void campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::setElement(size_t position, const ElementType& value) {
        cgtAssert(position >= 0 && position < getNumElements(), "Position out of bounds!");
        _data[position] = value;

    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    void campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::setElement(const cgt::svec3& position, const ElementType& value) {
        _data[_parent->positionToIndex(position)] = value;
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    typename campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::ElementType* campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getImageData() {
        return _data;
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    const typename campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::ElementType* campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getImageData() const {
        return _data;
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    typename campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::ElementType campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getElementLinear(const cgt::vec3 position) const {
        ElementType toReturn;
        for (size_t i = 0; i < NUMCHANNELS; ++i) {
            float channelValue = getElementNormalizedLinear(position, i);
            TypeTraits<BASETYPE, NUMCHANNELS>::setChannel(toReturn, i, TypeNormalizer::denormalizeFromFloat<BASETYPE>(channelValue));
        }
        return toReturn;
    }


    template<typename BASETYPE, size_t NUMCHANNELS>
    float campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getElementNormalizedLinear(const cgt::vec3& position, size_t channel) const {
        cgt::vec3 posAbs = cgt::max(position - 0.5f, cgt::vec3::zero);
        cgt::vec3 p = posAbs - floor(posAbs); // get decimal part
        cgt::svec3 llb = cgt::svec3(posAbs);
        cgt::svec3 urf = cgt::svec3(ceil(posAbs));
        urf = min(urf, getSize() - cgt::svec3(1)); // clamp so the lookups do not exceed the dimensions
        llb = min(llb, getSize() - cgt::svec3(1)); // dito

        /*
            interpolate linearly
        */
        return  getElementNormalized(cgt::svec3(llb.x, llb.y, llb.z), channel) * (1.f-p.x)*(1.f-p.y)*(1.f-p.z) // llB
              + getElementNormalized(cgt::svec3(urf.x, llb.y, llb.z), channel) * (    p.x)*(1.f-p.y)*(1.f-p.z) // lrB
              + getElementNormalized(cgt::svec3(urf.x, urf.y, llb.z), channel) * (    p.x)*(    p.y)*(1.f-p.z) // urB
              + getElementNormalized(cgt::svec3(llb.x, urf.y, llb.z), channel) * (1.f-p.x)*(    p.y)*(1.f-p.z) // ulB
              + getElementNormalized(cgt::svec3(llb.x, llb.y, urf.z), channel) * (1.f-p.x)*(1.f-p.y)*(    p.z) // llF
              + getElementNormalized(cgt::svec3(urf.x, llb.y, urf.z), channel) * (    p.x)*(1.f-p.y)*(    p.z) // lrF
              + getElementNormalized(cgt::svec3(urf.x, urf.y, urf.z), channel) * (    p.x)*(    p.y)*(    p.z) // urF
              + getElementNormalized(cgt::svec3(llb.x, urf.y, urf.z), channel) * (1.f-p.x)*(    p.y)*(    p.z);// ulF
    }

}

#endif // GENERICIMAGEREPRESENTATIONLOCAL_H__

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

#ifndef POINTPREDICATEHISTOGRAMPROPERTY_H__
#define POINTPREDICATEHISTOGRAMPROPERTY_H__


#include "core/properties/abstractproperty.h"
#include "modules/advancedusvis/datastructures/pointpredicatehistogram.h"

#include <tbb/atomic.h>
#include <memory>

namespace tgt {
    class Shader;
    class TextureUnit;
}

namespace campvis {
    /**
     * Property to wrap and access a PointPredicateHistogram.
     * 
     * Internally holds a pointer to a PointPredicateHistogram, when sharing this property with
     * others, they will share the pointer (instead of copying around as with other properties)!
     * Therefore, we use a std::shared_ptr.
     */
    class PointPredicateHistogramProperty : public AbstractProperty, public sigslot::has_slot {
    public:
        /**
         * Creates a new PointPredicateHistogramProperty
         * \param name      Property name (unchangable!)
         * \param title     Property title (e.g. used for GUI)
         */
        PointPredicateHistogramProperty(const std::string& name, const std::string& title);

        /**
         * Virtual Destructor
         **/
        virtual ~PointPredicateHistogramProperty();


        /// \see AbstractProperty::init
        virtual void init();
        /// \see AbstractProperty::deinit
        virtual void deinit();

        /// \see AbstractProperty::lock
        virtual void lock();
        /// \see AbstractProperty::unlock
        virtual void unlock();

        
        /**
         * Adds the given property \a prop to the set of shared properties.
         * The histogram of \a prop will be point to this property's histogram and no longer
         * own the pointer to it.
         * \note    Make sure not to build circular sharing or you will encounter endless loops!
         * \param   prop    Property to add.
         */
        virtual void addSharedProperty(AbstractProperty* prop);

        /**
         * Returns the predicate histogram.
         * Even though this is a non-const pointer, make sure not to mess with it!
         * \return  _histogram
         */
        PointPredicateHistogram* getPredicateHistogram();

        /**
         * Returns the predicate histogram in a aconst version.
         * \return  _histogram
         */
        const PointPredicateHistogram* getPredicateHistogram() const;


        /**
         * Returns a vector of all importances of the properties.
         * \return  A vector of all importances of the properties.
         */
        std::vector<float> getCurrentHistogramDistribution() const;

        /**
         * Adjusts all predicate importances using the given delta vector.
         * Each predicate's importance will be adjusted by the corresponding delta. Furthermore,
         * this method ensures to keep the predicate sum normalized to 1 and all predicate 
         * importances within [0, 1].
         * \note    Since we internally alter the deltas, the vector is passed by copy. This offers
         *          the compiler more room for optimizations.
         * \param   deltas          Vector of deltas describing the importnace adjustment for each predicate.
         * \param   baseHistogram   Base importance values to which the deltas are added
         * \param   fixedIndex      Optional index of a predicate that shall not be altered during invariant normalization, defaults to -1 (none).
         */
        void adjustImportances(std::vector<float> deltas, const std::vector<float>& baseHistogram, int fixedIndex = -1);


        /// Signal emitted when this predicate histogram's GLSL header has changed
        sigslot::signal0<> s_headerChanged;

    private:
        /// Slot called when a predicate histogram's configuration has changed
        void onHistogramConfigurationChanged();
        /// Slot called when a predicate histogram's header has changed
        void onHistogramHeaderChanged();

        /**
         * Sets the histogram pointer to \a histogram.
         * Cascades to shared properties.
         * \param   histogram   New pointer to the histogram
         */
        void setHistogramPointer(std::shared_ptr<PointPredicateHistogram> histogram);

        std::shared_ptr<PointPredicateHistogram> _histogram;
        tbb::atomic<size_t> _ignoreSignals;

        static const std::string loggerCat_;
    };

}

#endif // POINTPREDICATEHISTOGRAMPROPERTY_H__

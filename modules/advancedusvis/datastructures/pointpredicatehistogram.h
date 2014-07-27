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
#ifndef POINTPREDICATEHISTOGRAM_H__
#define POINTPREDICATEHISTOGRAM_H__

#include "sigslot/sigslot.h"

#include "modules/advancedusvis/datastructures/pointpredicate.h"

#include <vector>

namespace campvis {

    /**
     * Data object storing information about selected predicates and their settings.
     */
    class PointPredicateHistogram : public sigslot::has_slots<> {
    public:

        /**
         * Constructor.
         */
        PointPredicateHistogram();

        /**
         * Destructor.
         */
        virtual ~PointPredicateHistogram();

        /// Locks this histogram, so that changes are written to back buffer
        void lock();

        /// Unlocks this histogram
        void unlock();


        /**
         * Adds \a predicateToAdd to the predicate histogram.
         * \param   predicateToAdd  predicate to add
         */
        void addPredicate(AbstractPointPredicate* predicateToAdd);

        /**
         * Removes the predicate at index \a i.
         * \param   i   Index in predicate vector.
         */
        void removePredicate(size_t i);

        /**
         * Resets the predicate histogram to default setting.
         */
        void resetPredicates();

        /**
         * Returns the vector of predicates.
         * \return  _predicates
         */
        std::vector<AbstractPointPredicate*>& getPredicates();

        /**
         * Return the GLSL header defining all uniforms and GLSL function of the predicates.
         * \return  A GLSL header to include into all shaders using this predicate histogram.
         */
        std::string getGlslHeader() const;

        /**
         * Sets up \a shader for rendering (i.e. sets all uniform values).
         * \param   shader  Shader to setup.
         */
        void setupRenderShader(tgt::Shader* shader) const;


        /// Signal emitted when this predicate histogram's configuration (importance, color, ...) has changed
        sigslot::signal0<> s_configurationChanged;
        /// Signal emitted when this predicate histogram's GLSL header has changed
        sigslot::signal0<> s_headerChanged;

    private:
        /// Slot called when a predicate's configuration has changed
        void onPredicateConfigurationChanged();
        /// Slot called when a predicate's enabled state has changed
        void onPredicateEnabledChanged();

        /// vector of all voxel predicates
        std::vector<AbstractPointPredicate*> _predicates;

        static const std::string loggerCat_;

    };

}

#endif // POINTPREDICATEHISTOGRAM_H__

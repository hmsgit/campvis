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

#ifndef PREDICATEVOLUMEEXPLORER_H__
#define PREDICATEVOLUMEEXPLORER_H__

#include "modules/modulesapi.h"
#include "modules/vis/processors/volumeexplorer.h"
#include "modules/advancedusvis/processors/pointpredicateraycaster.h"
#include "modules/advancedusvis/processors/pointpredicateevaluator.h"


namespace campvis {
    /**
     * Combines a volume raycaster and 3 slice views for explorative volume visualization.
     */
    class CAMPVIS_MODULES_API PredicateVolumeExplorer : public VolumeExplorer {
        /// voxel type of the bitmask image (hardcoding this is not the most beautiful design though...)
        typedef uint8_t BitmaskType;

    public:
        /**
         * Constructs a new VolumeExplorer Processor
         * \param   viewportSizeProp    Pointer to the property defining the viewport size, must not be 0.
         * \param   raycaster           Raycaster to use for rendering, must not be 0, VolumeRenderer will take ownership.
         **/
        PredicateVolumeExplorer(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~PredicateVolumeExplorer();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "PredicateVolumeExplorer"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Combines a volume raycaster and 3 slice views for explorative volume visualization."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };
        
        /// \see cgt::EventListener::onEvent()
        virtual void onEvent(cgt::Event* e);

        DataNameProperty p_inputLabels;
        DataNameProperty p_inputSnr;
        DataNameProperty p_inputVesselness;
        DataNameProperty p_inputConfidence;

        PointPredicateHistogramProperty p_histogram;
        MetaProperty p_pbProperties;

    protected:
        /// Additional invalidation levels for this processor.
        /// Not the most beautiful design though.
        enum ProcessorInvalidationLevel {
            BITSET_INVALID = SCRIBBLE_INVALID << 1
        };

        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);

        /**
         * Slot getting called when one of the observed processors got invalidated.
         * Invalidates this meta-processor with the corresponding level.
         * \param   processor   The processor that emitted the signal
         */
        virtual void onProcessorInvalidated(AbstractProcessor* processor);

        /// \see VisualizationProcessor::onPropertyChanged
        virtual void onPropertyChanged(const AbstractProperty* prop);

        /**
         * Updates the point predicate histogram from the current scribble geometry.
         */
        void updatePredicateHistogramFromScribbles();

        /**
         * Returns the predicate bit histogram of the given vector of voxels.
         * \param   voxels  List of voxels to compute the bit histogram for.
         * \return  A histogram where each bit corresponds to the number of voxels in \a voxels where the corresponding predicate yields true.
         */
        std::vector<int> computeBitHistogram(const std::vector<cgt::vec3>& voxels);

        PointPredicateEvaluator _predicateEvaluation;   ///< Processor to perform the point predicate evaluation on.
        DataHandle _bitmaskHandle;                      ///< DataHandle storing the predicate bitmask

    private:
        static const std::string loggerCat_;
    };

}

#endif // PREDICATEVOLUMEEXPLORER_H__

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

#include "fibertracker.h"

#include <tbb/tbb.h>
#include <tbb/spin_mutex.h>

#include "core/datastructures/imagerepresentationlocal.h"
#include "modules/dti/datastructures/fiberdata.h"
#include <deque>

namespace campvis {
namespace dti {

    const std::string FiberTracker::loggerCat_ = "CAMPVis.modules.io.FiberTracker";

    class ApplyFiberTracking {
    public:
        ApplyFiberTracking(const ImageRepresentationLocal* input, const std::vector<cgt::vec3>& seeds, FiberData* output, int numSteps, float stepSize, float strainThreshold, float maximumAngle, tbb::spin_mutex& mutex)
            : _input(input)
            , _seeds(seeds)
            , _output(output)
            , _numSteps(numSteps)
            , _stepSize(stepSize)
            , _strainThreshold(strainThreshold * strainThreshold)
            , _maxAngle(maximumAngle)
            , _mutex(mutex)
        {
            _voxelSize = cgt::length(_input->getParent()->getMappingInformation().getVoxelSize());
        }
        
        /**
         * Retrieves a vec3 from \a vol using trilinear interpolation.
         * \param   position        voxel position
         **/
        inline cgt::vec3 getVec3FloatLinear(const cgt::vec3& position) const {
            cgt::vec3 result;
            result.x = _input->getElementNormalizedLinear(position, 0);
            result.y = _input->getElementNormalizedLinear(position, 1);
            result.z = _input->getElementNormalizedLinear(position, 2);
            return result;
        }
        
        /**
         * Checks whether the angle between \a a and \a b is lower that the given threshold.
         *
         * \param   a   direction of first tangent vector in world coordinates
         * \param   b   direction of second tangent vector in world coordinates
         *
         * \return  true, if angle is below the threshold.
         **/
        inline bool testTortuosity(const cgt::vec3& a, const cgt::vec3& b) const {
            float angle = fabs(acos(cgt::dot(a, b) / (cgt::length(a) * cgt::length(b))));
            return (angle < _maxAngle);
        }

        /**
         * Checks whether \a position is within volume bounds.
         *
         * \param   position    position in voxel coordinates
         *
         * true, if \a position is within bounds of eigenvalues volume.
         **/
        inline bool testBounds(const cgt::vec3& position) const {
            const cgt::svec3& dim = _input->getParent()->getSize();
            cgt::svec3 pos(cgt::ceil(position));
            return (cgt::hand(cgt::greaterThanEqual(pos, cgt::svec3::zero)) && cgt::hand(cgt::lessThanEqual(pos, dim)));
        }
        /**
         * Performs fiber tracking of a single fiber to a single direction starting at \a worldPosition 
         * and stores path in \a result. \a result will NOT contain the start point \a worldPosition.
         *
         * \param   worldPosition   start position in world coordinates
         * \param   forwards        flag whether to propagate forwards or backwards from \a worldPosition
         * \param   maxAngle        maximum angle between two fiber points to continue tracking
         * \param   result          fiber points will be stored in here - forward tracking will append, backward tracking push front
         **/
        void performSingleTracking(cgt::vec3 worldPosition, bool forwards, std::deque<cgt::vec3>& result) const {
            const cgt::mat4& WtV = _input->getParent()->getMappingInformation().getWorldToVoxelMatrix();

            cgt::vec3 voxelPosition = (WtV * cgt::vec4(worldPosition, 1.f)).xyz();
            cgt::vec3 direction = getVec3FloatLinear(voxelPosition);
            if (! forwards)
                direction *= -1.f;

            for (int i = 0; i < _numSteps; ++i) {
                // apply second order runge-kutta integration (Heun method)
                cgt::vec3 dir1 = getVec3FloatLinear((WtV * cgt::vec4(worldPosition, 1.f)).xyz()) * _stepSize * _voxelSize;
                if (cgt::dot(direction, dir1) < 0)
                    dir1 *= -1.f;

                cgt::vec3 dir2 = getVec3FloatLinear((WtV * cgt::vec4(worldPosition + dir1, 1.f)).xyz()) * _stepSize * _voxelSize;
                if (cgt::dot(direction, dir2) < 0)
                    dir2 *= -1.f;

                cgt::vec3 vProp = (dir1 + dir2) * .5f;
                worldPosition += vProp;
                voxelPosition = (WtV * cgt::vec4(worldPosition, 1.f)).xyz();

                // check termination criteria
                if (cgt::lengthSq(vProp) < _strainThreshold || !testBounds(voxelPosition) || !testTortuosity(direction, vProp))
                    break;

                direction = vProp;
                if (forwards)
                    result.push_back(worldPosition);
                else
                    result.push_front(worldPosition);
            }
        }

        void operator() (const tbb::blocked_range<size_t>& range) const {
            for (size_t i = range.begin(); i != range.end(); ++i) {
                const cgt::vec3& position = _seeds[i];

                // perform fiber tracking in both directions
                std::deque<cgt::vec3> vertices;
                performSingleTracking(position, false, vertices);
                vertices.push_back(position);
                performSingleTracking(position, true, vertices);

                if (vertices.size() > 1) {
                    tbb::spin_mutex::scoped_lock lock(_mutex);
                    _output->addFiber(vertices);
                }
            }
        }

    protected:
        const ImageRepresentationLocal* _input;
        const std::vector<cgt::vec3>& _seeds;
        FiberData* _output;
        int _numSteps;
        float _stepSize;
        float _voxelSize;
        float _strainThreshold;
        float _maxAngle;

        tbb::spin_mutex& _mutex;
    };

    FiberTracker::FiberTracker() 
        : AbstractProcessor()
        , p_strainId("StrainId", "Input Strain Data", "input", DataNameProperty::READ)
        , p_outputID("OutputId", "Output Fiber Data", "output", DataNameProperty::WRITE)
        , p_updateButton("UpdateButton", "Perform Tracking")
        , p_seedDistance("SeedDistance", "Seed Distance", 4, 1, 16, 1)
        , p_numSteps("NumSteps", "Maximum Number of Steps", 256, 16, 1024, 1)
        , p_stepSize("StepSize", "Base Step Size", 1.f, .01f, 10.f, 0.01f, 2)
        , p_strainThreshold("StrainThreshold", "Local Strain Threshold", .5f, .1f, 1.f, 0.01f, 2)
        , p_maximumAngle("MaximumAngle", "Maxium Angle", 25.f, 0.f, 100.f, 0.1f, 2)
    {
        addProperty(p_strainId, VALID);
        addProperty(p_outputID, VALID);
        addProperty(p_updateButton);
        addProperty(p_seedDistance, VALID);
        addProperty(p_numSteps, VALID);
        addProperty(p_stepSize, VALID);
        addProperty(p_strainThreshold, VALID);
        addProperty(p_maximumAngle, VALID);

    }

    FiberTracker::~FiberTracker() {

    }

    void FiberTracker::updateResult(DataContainer& data) {
        ImageRepresentationLocal::ScopedRepresentation strainData(data, p_strainId.getValue());

        if (strainData != 0) {
            if (strainData.getImageData()->getNumChannels() == 3) {
                LDEBUG("Generating seeds...");
                std::vector<cgt::vec3> seeds = performUniformSeeding(*strainData);

                LDEBUG("Generating fibers...");
                FiberData* fibers = new FiberData();
                tbb::spin_mutex mutex;
                tbb::parallel_for(
                    tbb::blocked_range<size_t>(0, seeds.size()), 
                    ApplyFiberTracking(strainData, seeds, fibers, p_numSteps.getValue(), p_stepSize.getValue(), p_strainThreshold.getValue(), p_maximumAngle.getValue(), mutex));

                LDEBUG("done.");

                data.addData(p_outputID.getValue(), fibers);
            }
            else {
                LERROR("Wrong number of channels.");
            }
        }
        else {
            LERROR("No input data.");
        }
    }

    std::vector<cgt::vec3> FiberTracker::performUniformSeeding(const ImageRepresentationLocal& strainData) const {
        std::vector<cgt::vec3> seeds;
        const cgt::mat4& VtW = strainData.getParent()->getMappingInformation().getVoxelToWorldMatrix();
        float threshold = p_strainThreshold.getValue() * p_strainThreshold.getValue();
        int inc = p_seedDistance.getValue();

        for (size_t z = 0; z < strainData.getSize().z; z += inc) {
            for (size_t y = 0; y < strainData.getSize().y; y += inc) {
                for (size_t x = 0; x < strainData.getSize().x; x += inc) {
                    cgt::vec3 pos = cgt::vec3(float(x), float(y), float(z));
                    if (cgt::lengthSq(getVec3FloatLinear(strainData, pos)) > threshold) {
                        seeds.push_back((VtW * cgt::vec4(pos, 1.f)).xyz());
                    }
                }
            }
        }

        return seeds;
    }

    cgt::vec3 FiberTracker::getVec3FloatLinear(const ImageRepresentationLocal& strainData, const cgt::vec3& position) const {
        cgt::vec3 result;
        result.x = strainData.getElementNormalizedLinear(position, 0);
        result.y = strainData.getElementNormalizedLinear(position, 1);
        result.z = strainData.getElementNormalizedLinear(position, 2);
        return result;
    }

}
}

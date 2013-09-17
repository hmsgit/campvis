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

#ifndef STRAINFIBERTRACKER_H__
#define STRAINFIBERTRACKER_H__

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/buttonproperty.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/numericproperty.h"

#include <deque>

namespace campvis {
    class ImageRepresentationLocal;

    /**
     * Performs a fiber tracking of incomoing strain data.
     */
    class StrainFiberTracker : public AbstractProcessor {
    public:
        /**
         * Constructs a new StrainFiberTracker Processor
         **/
        StrainFiberTracker();

        /**
         * Destructor
         **/
        virtual ~StrainFiberTracker();


        /**
         * Reads the raw file into an ImageRepresentationDisk representation
         * \param data  DataContainer to work on
         */
        virtual void process(DataContainer& data);

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "StrainFiberTracker"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Performs a fiber tracking of incomoing strain data."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual const ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        DataNameProperty p_strainId;   ///< image ID for input strain data
        DataNameProperty p_outputID;   ///< image ID for output fiber data

        ButtonProperty p_updateButton;      ///< Button to start tracking

        IntProperty p_seedDistance;

        IntProperty p_numSteps;             ///< Maximum number of steps per fiber
        FloatProperty p_stepSize;           ///< Base step size

        FloatProperty p_strainThreshold;    ///< Local strain threshold (minimum) to perform tracking
        FloatProperty p_maximumAngle;       ///< maximum angle between two adjacent fiber segments

    protected:
        /**
         * Creates seed points uniformly spread over volume and writes them to \a seeds.
         * \param   strainData  Input strain data
         * \return  vector of seed points in voxel coordinates
         **/
        std::vector<tgt::vec3> performUniformSeeding(const ImageRepresentationLocal& strainData) const;

        /**
         * Retrieves a vec3 from \a vol using trilinear interpolation.
         *
         * \param   vol             volume to get data from
         * \param   position        voxel position
         **/
        inline tgt::vec3 getVec3FloatLinear(const ImageRepresentationLocal& strainData, const tgt::vec3& position) const;

        static const std::string loggerCat_;
    };

}

#endif // STRAINFIBERTRACKER_H__

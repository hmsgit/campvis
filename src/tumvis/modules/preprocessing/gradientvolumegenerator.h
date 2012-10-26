// ================================================================================================
// 
// This file is part of the CAMPVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
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

#ifndef GRADIENTVOLUMEGENERATOR_H__
#define GRADIENTVOLUMEGENERATOR_H__

#include <string>

#include "core/classification/abstracttransferfunction.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/cameraproperty.h"

namespace campvis {
    /**
     * Creates the gradient volume for the given intensity volume.
     */
    class GradientVolumeGenerator : public AbstractProcessor {
    public:
        /**
         * Constructs a new GradientVolumeGenerator Processor
         **/
        GradientVolumeGenerator();

        /**
         * Destructor
         **/
        virtual ~GradientVolumeGenerator();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "GradientVolumeGenerator"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Creates the gradient volume for the given intensity volume."; };

        virtual void process(DataContainer& data);

        DataNameProperty _inputVolume;      ///< ID for input volume
        DataNameProperty _outputGradients;  //< ID for output gradient volume


    protected:

        static const std::string loggerCat_;
    };

}

#endif // GRADIENTVOLUMEGENERATOR_H__

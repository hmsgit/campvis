// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#ifndef LHHISTOGRAM_H__
#define LHHISTOGRAM_H__

#include <string>

#include "core/classification/abstracttransferfunction.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/cameraproperty.h"

namespace TUMVis {
    /**
     * Creates Lookup volumes vor generation LH-Histograms of volumes as well as the LH histogram.
     */
    class LHHistogram : public AbstractProcessor {
    public:
        /**
         * Constructs a new LHHistogram Processor
         **/
        LHHistogram();

        /**
         * Destructor
         **/
        virtual ~LHHistogram();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "LHHistogram"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Creates Lookup volumes vor generation LH-Histograms of volumes as well as the LH histogram."; };

        virtual void process(DataContainer& data);

        DataNameProperty _inputVolume;      ///< ID for input volume
        DataNameProperty _inputGradients;   ///< ID for input gradient volume

        DataNameProperty _outputFH;         ///< ID for output FH volume
        DataNameProperty _outputFL;         ///< ID for output FL volume

    protected:

        static const std::string loggerCat_;
    };

}

#endif // LHHISTOGRAM_H__

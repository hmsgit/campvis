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

#include "processordecoratorgradient.h"

#include "tgt/shadermanager.h"
#include "core/properties/propertycollection.h"

namespace campvis {
    static const GenericOption<ProcessorDecoratorGradient::GradientMethod> gradientOptions[4] = {
        GenericOption<ProcessorDecoratorGradient::GradientMethod>("Forward", "Forward Differences", ProcessorDecoratorGradient::ForwardDifferences),
        GenericOption<ProcessorDecoratorGradient::GradientMethod>("Central", "Central Differences", ProcessorDecoratorGradient::CentralDifferences),
        GenericOption<ProcessorDecoratorGradient::GradientMethod>("Sobel", "Sobel Filter", ProcessorDecoratorGradient::SobelFilter),
        GenericOption<ProcessorDecoratorGradient::GradientMethod>("FilteredCentral", "Filtered Central Differences", ProcessorDecoratorGradient::FilteredCentralDifferences)
    };

    ProcessorDecoratorGradient::ProcessorDecoratorGradient()
        : AbstractProcessorDecorator()
        , _gradientMethod("GradientMethod", "Gradient Computation Method", gradientOptions, 4, AbstractProcessor::INVALID_SHADER | AbstractProcessor::INVALID_RESULT)
    {
        _gradientMethod.setValue(1);
    }

    ProcessorDecoratorGradient::~ProcessorDecoratorGradient() {
    }

    void ProcessorDecoratorGradient::addProperties(HasPropertyCollection* propCollection) {
        propCollection->addProperty(&_gradientMethod);
    }

    std::string ProcessorDecoratorGradient::generateHeader() const {
        std::string toReturn;
        switch (_gradientMethod.getOptionValue()) {
            case ForwardDifferences:
                toReturn.append("#define computeGradient(tex, texParams, texCoords) computeGradientForwardDifferences(tex, texParams, texCoords)\n");
                break;
            case CentralDifferences:
                toReturn.append("#define computeGradient(tex, texParams, texCoords) computeGradientCentralDifferences(tex, texParams, texCoords)\n");
                break;
            case FilteredCentralDifferences:
                toReturn.append("#define computeGradient(tex, texParams, texCoords) computeGradientFilteredCentralDifferences(tex, texParams, texCoords)\n");
                break;
            case SobelFilter:
                toReturn.append("#define computeGradient(tex, texParams, texCoords) computeGradientSobel(tex, texCoords)\n");
                break;
            default:
                tgtAssert(false, "Invalid enum!");
                break;
        }

        return toReturn;
    }

}

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

#include "processordecoratormasking.h"

#include "tgt/shadermanager.h"
#include "core/properties/propertycollection.h"

namespace campvis {

    ProcessorDecoratorMasking::ProcessorDecoratorMasking()
        : AbstractProcessorDecorator()
        , _applyMask("applyMask", "Apply Mask to image", false, AbstractProcessor::INVALID_SHADER)
        , _maskID("maskID", "Mask Image ID", "mask", DataNameProperty::READ)
        , _maskColor("maskColor", "Mask Color", tgt::vec4(0.f), tgt::vec4(0.f), tgt::vec4(1.f), tgt::vec4(0.01f))
        , _texUnit(0)
        , _maskImage(0)
    {
    }

    ProcessorDecoratorMasking::~ProcessorDecoratorMasking() {
        delete _texUnit;
        delete _maskImage;
    }

    void ProcessorDecoratorMasking::addProperties(HasPropertyCollection* propCollection) {
        propCollection->addProperty(&_applyMask);
        propCollection->addProperty(&_maskID);
        propCollection->addProperty(&_maskColor);
    }

    void ProcessorDecoratorMasking::renderProlog(const DataContainer& dataContainer, tgt::Shader* shader) {
        _texUnit = new tgt::TextureUnit();
        if (_applyMask.getValue()) {
            shader->setUniform("_maskColor", _maskColor.getValue());

            _maskImage = new DataContainer::ScopedTypedData<RenderData>(dataContainer, _maskID.getValue());
            if (*_maskImage != 0) {
                (*_maskImage)->bindColorTexture(shader, *_texUnit, "_maskImage", "_maskTexParams");
            }
            else {
                LERRORC("CAMPVis.core.ProcessorDecoratorMasking", "No valid mask image found!");
            }
        }
    }

    void ProcessorDecoratorMasking::renderEpilog(tgt::Shader* shader) {
        delete _texUnit;
        _texUnit = 0;

        delete _maskImage;
        _maskImage = 0;
    }

    std::string ProcessorDecoratorMasking::generateHeader() const {
        if (_applyMask.getValue())
            return "#define APPLY_MASK 1\n";
        else
            return "";
    }

}

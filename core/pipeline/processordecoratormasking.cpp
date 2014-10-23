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

#include "processordecoratormasking.h"

#include "cgt/shadermanager.h"
#include "core/pipeline/abstractprocessor.h"

namespace campvis {

    ProcessorDecoratorMasking::ProcessorDecoratorMasking()
        : AbstractProcessorDecorator()
        , _applyMask("applyMask", "Apply Mask to image", false)
        , _maskID("maskID", "Mask Image ID", "mask", DataNameProperty::READ)
        , _maskColor("maskColor", "Mask Color", tgt::vec4(0.f), tgt::vec4(0.f), tgt::vec4(1.f))
        , _texUnit(0)
        , _maskImage(0)
    {
        _applyMask.s_changed.connect(this, &ProcessorDecoratorMasking::onPropertyChanged);
        _maskID.setVisible(false);
        _maskColor.setVisible(false);
    }

    ProcessorDecoratorMasking::~ProcessorDecoratorMasking() {
        _applyMask.s_changed.disconnect(this);
        delete _texUnit;
        delete _maskImage;
    }

    void ProcessorDecoratorMasking::addProperties(AbstractProcessor* propCollection) {
        propCollection->addProperty(_applyMask, AbstractProcessor::INVALID_SHADER);
        propCollection->addProperty(_maskID);
        propCollection->addProperty(_maskColor);
    }

    void ProcessorDecoratorMasking::renderProlog(const DataContainer& dataContainer, tgt::Shader* shader) {
        _texUnit = new tgt::TextureUnit();
        if (_applyMask.getValue()) {
            shader->setUniform("_maskColor", _maskColor.getValue());

            _maskImage = new ScopedTypedData<RenderData>(dataContainer, _maskID.getValue());
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

    void ProcessorDecoratorMasking::onPropertyChanged(const AbstractProperty* p) {
        if (p == &_applyMask) {
            _maskID.setVisible(_applyMask.getValue());
            _maskColor.setVisible(_applyMask.getValue());
        }
    }

}

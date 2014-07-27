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

#include "processordecoratorbackground.h"

#include "tgt/shadermanager.h"
#include "core/properties/propertycollection.h"

namespace campvis {

    ProcessorDecoratorBackground::ProcessorDecoratorBackground()
        : AbstractProcessorDecorator()
        , _backgroundColor1("backgroundColor1", "Background Color 1", tgt::vec4(.9f, .9f, .9f, 1), tgt::vec4(0.f), tgt::vec4(1.f))
        , _backgroundColor2("backgroundColor2", "Background Color 2", tgt::vec4(.6f, .6f, .6f, 1), tgt::vec4(0.f), tgt::vec4(1.f))
    {
    }

    ProcessorDecoratorBackground::~ProcessorDecoratorBackground() {

    }

    void ProcessorDecoratorBackground::addProperties(HasPropertyCollection* propCollection) {
        propCollection->addProperty(_backgroundColor1);
        propCollection->addProperty(_backgroundColor2);
    }

    void ProcessorDecoratorBackground::renderProlog(const DataContainer& dataContainer, tgt::Shader* shader) {
        shader->setUniform("_backgroundColor1", _backgroundColor1.getValue());
        shader->setUniform("_backgroundColor2", _backgroundColor2.getValue());
    }

}

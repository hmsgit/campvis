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

#ifndef PROCESSORDECORATORMASKING_H__
#define PROCESSORDECORATORMASKING_H__

#include "tgt/textureunit.h"
#include "core/datastructures/datacontainer.h"
#include "core/datastructures/renderdata.h"
#include "core/pipeline/abstractprocessordecorator.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/floatingpointproperty.h"

namespace campvis {

    class CAMPVIS_CORE_API ProcessorDecoratorMasking : public AbstractProcessorDecorator, public sigslot::has_slots {
    public:
        ProcessorDecoratorMasking();
        virtual ~ProcessorDecoratorMasking();

    protected:
        void addProperties(AbstractProcessor* propCollection);

        void renderProlog(const DataContainer& dataContainer, tgt::Shader* shader);

        void renderEpilog(tgt::Shader* shader);

        std::string generateHeader() const;

        BoolProperty _applyMask;            ///< Flag whether to apply mask
        DataNameProperty _maskID;           ///< ID for mask image (optional)
        Vec4Property _maskColor;            ///< Mask color

        tgt::TextureUnit* _texUnit;
        ScopedTypedData<RenderData>* _maskImage;

    private:
        void onPropertyChanged(const AbstractProperty* p);

    };

}

#endif // PROCESSORDECORATORMASKING_H__

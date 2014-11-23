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

#ifndef GEOMETRY2DTRANSFERFUNCTIONLUA_H__
#define GEOMETRY2DTRANSFERFUNCTIONLUA_H__

#include "tbb/mutex.h"
#include "cgt/painter.h"
#include "cgt/event/eventlistener.h"
#include "core/classification/tfgeometry2d.h"
#include "abstracttransferfunctionlua.h"

class QGridLayout;
class QLabel;
class QPushButton;

namespace tgt {
    class QtThreadedCanvas;
    class Shader;
}

namespace campvis {
    class ColorPickerWidget;
    class Geometry2DTransferFunction;
    class AbstractTFGeometryManipulator;
    class WholeTFGeometryManipulator;

    /**
     * Lua generator for a Geometry2DTransferFunction.
     */
    class Geometry2DTransferFunctionLua : public AbstractTransferFunctionLua {

    public:
        /**
         * Creates a new lua generator for the TransferFunctionProperty \a property.
         * \param   prop        TransferFunctionProperty to generate the lua for.
         * \param   tf          The transfer function the lua shall handle.
         * \param   parent      Parent Qt widget
         */
        Geometry2DTransferFunctionLua(TransferFunctionProperty* prop, Geometry2DTransferFunction* tf);

        /**
         * Destructor
         */
        virtual ~Geometry2DTransferFunctionLua();

        std::string getLuaScript(std::string propNamePrefix, std::string luaProc);

    };
}

#endif // GEOMETRY2DTRANSFERFUNCTIONLUA_H__

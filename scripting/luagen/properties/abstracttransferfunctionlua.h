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

#ifndef ABSTRACTTRANSFERFUNCTIONLUA_H__
#define ABSTRACTTRANSFERFUNCTIONLUA_H__


#include "core/properties/transferfunctionproperty.h"


namespace campvis {
    class AbstractProperty;
    class AbstractTransferFunction;

    /**
     * Abstract base class for transfer function lua generators.
     */
    class AbstractTransferFunctionLua {

    public:
        /**
         * Creates a new transfer function lua for the for the AbstractTransferFunction \a tf.
         * \param   prop        TransferFunctionProperty to generate the lua script for.
         * \param   tf          The transfer function the lua script shall handle.
         */
        AbstractTransferFunctionLua(TransferFunctionProperty* prop, AbstractTransferFunction* tf);

        /**
         * Destructor
         */
        virtual ~AbstractTransferFunctionLua();

        virtual std::string getLuaScript(std::string propNamePrefix, std::string luaProc) = 0;

    protected:
        const TransferFunctionProperty::IntensityHistogramType* getIntensityHistogram() const;

        TransferFunctionProperty* _tfProperty;          ///< The parent TransferFunctionProperty of this editor
        AbstractTransferFunction* _transferFunction;    ///< The transfer function this lua handles

    private:
    };
}

#endif // ABSTRACTTRANSFERFUNCTIONLUA_H__

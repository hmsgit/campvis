// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#ifndef TRANSFERFUNCTIONEDITORFACTORY_H__
#define TRANSFERFUNCTIONEDITORFACTORY_H__

#include "application/applicationapi.h"

namespace campvis {
    class AbstractTransferFunction;
    class AbstractTransferFunctionEditor;
    class TransferFunctionProperty;

    /**
     * Factory class offering the static method createEditor to create transfer function editors
     * for a given transfer function (depending on its type).
     */
    class CAMPVIS_APPLICATION_API TransferFunctionEditorFactory {
    public:
        /**
         * Creates the corresponding TransferFunctionEditor for the given transfer function \a tf.
         * \note    The callee has to take the ownership of the returned pointer.
         * \param   prop    TransferFunctionProperty to generate the editor for.
         * \return  A new transfer function editor for the given transfer function (depending on its type).
         */
        static AbstractTransferFunctionEditor* createEditor(TransferFunctionProperty* prop);

    private:
        /// Shall not instantiate
        TransferFunctionEditorFactory();
    };
}

#endif // TRANSFERFUNCTIONEDITORFACTORY_H__

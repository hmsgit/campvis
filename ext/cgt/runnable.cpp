/**********************************************************************
 *                                                                    *
 * cgt - CAMP Graphics Toolbox, Copyright (C) 2012-2014               *
 *     Chair for Computer Aided Medical Procedures                    *
 *     Technische Universitaet Muenchen, Germany.                     *
 *     <http://campar.in.tum.de/>                                     *
 *                                                                    *
 * forked from tgt - Tiny Graphics Toolbox, Copyright (C) 2006-2011   *
 *     Visualization and Computer Graphics Group, Department of       *
 *     Computer Science, University of Muenster, Germany.             *
 *     <http://viscg.uni-muenster.de>                                 *
 *                                                                    *
 * This file is part of the cgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

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

#include "runnable.h"

namespace cgt {
    namespace {
        void invokeThread(Runnable* r) {
            r->run();
        }
    }

    Runnable::Runnable() 
        : _stopExecution()
        , _thread(0) 
    {
        _stopExecution = false;
        _running = false;
    }

    Runnable::~Runnable() {
        if (_running)
            stop();

        delete _thread;
    }

    void Runnable::stop() {
        if (!_running || _thread == 0)
            return;

        _stopExecution = true; 
        try { 
            if (_thread->joinable())
                _thread->join(); 

            _running = false;
        } 
        catch(std::exception& e) { 
            LERRORC("CAMPVis.core.tools.Runnable", "Caught exception during _thread.join: " << e.what());
        } 
    }

    void Runnable::start() { 
        _thread = new std::thread(&invokeThread, this);
        _running = true;
    }
}


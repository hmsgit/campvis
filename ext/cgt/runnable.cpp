/**********************************************************************
 *                                                                    *
 * cgt - CAMP Graphics Toolbox, Copyright (C) 2012-2015               *
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

#include "runnable.h"

namespace cgt {
    void invokeThread(Runnable* r) {
        r->run();
        r->_running = false;
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
        if (_thread == 0)
            return;

        _stopExecution = true; 
        try { 
            if (_thread->joinable())
                _thread->join(); 
        } 
        catch(std::exception& e) { 
            LERRORC("CAMPVis.core.tools.Runnable", "Caught exception during _thread.join: " << e.what());
        } 
    }

    void Runnable::start() { 
        _thread = new std::thread(&invokeThread, this);
        _running = true;
    }

// ================================================================================================
    
    RunnableWithConditionalWait::RunnableWithConditionalWait() 
        : Runnable()
    {}

    RunnableWithConditionalWait::~RunnableWithConditionalWait() {

    }

    void RunnableWithConditionalWait::stop() {
        while (_running) {
            _stopExecution = true;
            _evaluationCondition.notify_all();
            std::this_thread::yield();
        }

        Runnable::stop();
    }
}


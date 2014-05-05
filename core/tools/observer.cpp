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

#include "observer.h"

namespace campvis {

    Observer::Observer() {

    }

    Observer::~Observer() {

    }

    Observable::Observable() {

    }

    Observable::~Observable() {

    }

    void Observable::addObserver(Observer* o) const {
        tbb::spin_mutex::scoped_lock lock(_localMutex);
        _observers.insert(o);
    }

    void Observable::removeObserver(Observer* o) const {
        tbb::spin_mutex::scoped_lock lock(_localMutex);
        _observers.erase(o);
    }

    void Observable::notifyObservers() const {
        tbb::spin_mutex::scoped_lock lock(_localMutex);
        for (std::set<Observer*>::iterator it = _observers.begin(); it != _observers.end(); ++it) {
            (*it)->onNotify();
        }
    }
}

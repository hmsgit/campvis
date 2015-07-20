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

#include "bufferinglog.h"

#include "cgt/assert.h"

namespace campvis {

    BufferingLog::BufferingLog(unsigned int maxSize, QObject* parent, bool dateStamping,
                               bool timeStamping, bool showCat, bool showLevel)
        : QObject(parent)
    {
        _maxSize = maxSize;
        dateStamping_ = dateStamping;
        timeStamping_ = timeStamping;
        showCat_ = showCat;
        showLevel_ = showLevel;
    }

    void BufferingLog::logFiltered(const std::string& cat, cgt::LogLevel level, const std::string& msg,
                                   const std::string& /*extendedInfo*/)
    {
        std::string output = "";

        if (dateStamping_)
            output += "[" + getDateString() + "] ";
        if (timeStamping_)
            output += "[" + getTimeString() + "] ";
        if (showCat_)
            output += cat + " ";
        if (showLevel_)
            output += "(" + getLevelString(level) + ") ";
        if (output != "")
            output += '\t';

        output += msg;

        {
            tbb::mutex::scoped_lock lock(_localMutex);
            _buffer.push_back(output);

            if (_buffer.size() > _maxSize) {
                _buffer.pop_front();
            }
        }

        emit s_messageAppended(QString::fromStdString(output), level);
    }
}

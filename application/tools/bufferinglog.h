// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#ifndef BUFFERINGLOG_H__
#define BUFFERINGLOG_H__

#include "tbb/mutex.h"
#include "tgt/logmanager.h"

#include <deque>
#include <QObject>

namespace campvis {

    /**
     * BufferingLog stores up to the specified number of messages in memory.
     */
    class BufferingLog : public QObject, public tgt::Log {

        Q_OBJECT

    public:
        /**
         * Creates a new BufferingLog.
         *
         * When the BufferingLog reaches its maximum capacity and is asked to store
         * additional messages, it discards old ones according to the least recently
         * used policy.
         *
         * \param   maxSize          the maximum number of messages this log should store
         * \param   dateStamping     should logged messages be date-stamped
         * \param   timeStamping     should logged messages be time-stamped
         * \param   showCat          should the category be printed along with the messages
         * \param   showLevel        should the LogLevel be printed along with the messages
         */
        explicit BufferingLog(unsigned int maxSize, QObject* parent = 0, bool dateStamping = true,
                     bool timeStamping = true, bool showCat = true, bool showLevel = true);
        virtual ~BufferingLog() {}
        bool isOpen() { return true; }

    signals:
        /**
         * Signal invoked when a new message has been appended to the log.
         *
         * \param   message          the new log message
         * \param   level            Log level
         */
        void s_messageAppended(const QString& message, int level);

    protected:
        unsigned int _maxSize;
        tbb::mutex _localMutex;
        std::deque<std::string> _buffer;
        void logFiltered(const std::string &cat, tgt::LogLevel level, const std::string& msg, const std::string& extendedInfo="");
    };
}

#endif // BUFFERINGLOG_H__

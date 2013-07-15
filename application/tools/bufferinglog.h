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
        BufferingLog(unsigned int maxSize, QObject* parent = 0, bool dateStamping = true,
                     bool timeStamping = true, bool showCat = true, bool showLevel = true);
        virtual ~BufferingLog() {}
        bool isOpen() { return true; }

    signals:
        /**
         * Signal invoked when a new message has been appended to the log.
         *
         * \param   message          the new log message
         */
        void s_messageAppended(const QString& message);

    protected:
        unsigned int _maxSize;
        tbb::mutex _localMutex;
        std::deque<std::string> _buffer;
        void logFiltered(const std::string &cat, tgt::LogLevel level, const std::string& msg, const std::string& extendedInfo="");
    };
}

#endif // BUFFERINGLOG_H__

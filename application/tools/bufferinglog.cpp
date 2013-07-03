#include "bufferinglog.h"

#include "tgt/assert.h"

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

    void BufferingLog::logFiltered(const std::string& cat, LogLevel level, const std::string& msg,
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

        emit s_messageAppended(QString::fromStdString(output));
    }
}

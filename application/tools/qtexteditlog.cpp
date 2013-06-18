#include "qtexteditlog.h"

#include "tgt/assert.h"

namespace campvis {

    QTextEditLog::QTextEditLog(QTextEdit* logDisplay, bool dateStamping, bool timeStamping,
                               bool showCat, bool showLevel)
        : _logDisplay(logDisplay)
    {
        tgtAssert(logDisplay != 0, "Log display widget must not be 0.");

        dateStamping_ = dateStamping;
        timeStamping_ = timeStamping;
        showCat_ = showCat;
        showLevel_ = showLevel;

        connect(this, SIGNAL(s_messageReady(const QString&)),
                _logDisplay, SLOT(append(const QString&)));
    }

    void QTextEditLog::logFiltered(const std::string& cat, LogLevel level, const std::string& msg,
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

        emit s_messageReady(QString::fromStdString(output + msg));
    }
}
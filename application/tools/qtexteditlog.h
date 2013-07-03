#ifndef QTEXTEDITLOG_H__
#define QTEXTEDITLOG_H__

#include "tgt/logmanager.h"

#include <QObject>
#include <QTextEdit>
#include <QTextDocument>

using namespace tgt;

namespace campvis {

    /**
     * QTextEditLog implements logging to a QTextEdit instance.
     */
    class QTextEditLog : private QObject, public Log {

        Q_OBJECT

    public:
        /**
         * Creates a new QTextEditLog.
         *
         * QTextEditLog doesn't take ownership of the QTextEdit instance it's passed.
         *
         * \param   output           a QTextEdit instance to which messages will be logged
         * \param   parent           the log's parent object
         * \param   dateStamping     should logged messages be date-stamped
         * \param   timeStamping     should logged messages be time-stamped
         * \param   showCat          should the category be printed along with the messages
         * \param   showLevel        should the LogLevel be printed along with the messages
         */
        QTextEditLog(QTextEdit* logDisplay, QObject* parent = 0, bool dateStamping = true,
                     bool timeStamping = true, bool showCat = true, bool showLevel = true);
        virtual ~QTextEditLog() {}
        bool isOpen() { return true; }

	signals:
        /** Internal signal used to pass messages to _logDisplay in a thread-safe way.
         *
         * This class' \c log method is called from several non-GUI
         * threads. If it were to directly access its underlying
         * QTextEdit widget, which is neither thread-safe nor reentrant,
         * the application would break badly. However, if we connect
         * a signal to _logDisplay's append slot to display log
         * messages, Qt takes care of queueing slot accesses in the GUI
         * thread for us.
         */
        void s_messageReady(const QString& message);

    protected:
        QTextEdit* _logDisplay;
        void logFiltered(const std::string &cat, LogLevel level, const std::string &msg, const std::string &extendedInfo="");
    };
}

#endif // QTEXTEDITLOG_H__

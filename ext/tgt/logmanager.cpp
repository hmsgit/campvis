/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2011 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the tgt library. This library is free         *
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

#include "tgt/logmanager.h"

#include <ctime>
#include <stdio.h>

using namespace std;

namespace tgt {

bool Log::testFilter(const std::string &cat, LogLevel level) {
    for (size_t i = 0; i < filters_.size(); i++)     {
        if (filters_[i].children_) {
            if (cat.find(filters_[i].cat_, 0) == 0) {
                if (filters_[i].level_ <= level)
                    return true;
            }
        }
        else {
            if (filters_[i].cat_ == cat) {
                if (filters_[i].level_ <= level)
                    return true;
            }
        }
    }
    return false;
}

void Log::log(const std::string &cat, LogLevel level, const std::string &msg, const std::string &extendedInfo) {
    if (testFilter(cat, level))
        logFiltered(cat, level, msg, extendedInfo);
}

void Log::addCat(const std::string &cat, bool children, LogLevel level) {
    LogFilter newFilter;
    newFilter.cat_ = cat;
    newFilter.children_ = children;
    newFilter.level_ = level;
    filters_.push_back(newFilter);
}

std::string Log::getTimeString() {
    time_t long_time = 0;
    tm *now = 0;
    time(&long_time);
    now = localtime(&long_time);
    char SzBuffer[256];
    sprintf(SzBuffer, "%.2i:%.2i:%.2i", now->tm_hour, now->tm_min, now->tm_sec);
    string temp(SzBuffer);
    return temp;
}

std::string Log::getDateString() {
    time_t long_time = 0;
    tm *now = 0;
    time(&long_time);
    now = localtime(&long_time);
    char SzBuffer[256];
    sprintf(SzBuffer, "%.2i.%.2i.%.4i", now->tm_mday, now->tm_mon + 1, now->tm_year + 1900);
    string temp(SzBuffer);
    return temp;
}

std::string Log::getLevelString(LogLevel level) {
    switch (level) {
    case Debug: return "Debug";
    case Info: return "Info";
    case Warning: return "Warning";
    case Error: return "Error";
    case Fatal: return "Fatal";
    default: return "UNKNOWN";
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TextLog::logFiltered(const std::string& cat, LogLevel level, const std::string& msg,
                          const std::string& /*extendedInfo*/)
{
    if (!file_)
        return;

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
    fputs((output +  msg + "\n").c_str(), file_);
    fflush(file_);
}

TextLog::TextLog(const std::string &filename, bool dateStamping, bool timeStamping, bool showCat, bool showLevel) {
    std::string path = (LogMgr.getLogDir().empty() ? "" : LogMgr.getLogDir() + "/");
    file_ = fopen((path + filename).c_str(),"w");
    timeStamping_ = timeStamping;
    dateStamping_ = dateStamping;
    showCat_ = showCat;
    showLevel_ = showLevel;
}

TextLog::~TextLog() {
    if (!file_)
        return;
    fputs("---\n", file_);
    fclose(file_);
}

bool TextLog::isOpen() {
    return (file_ != 0);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ConsoleLog::ConsoleLog(bool dateStamping, bool timeStamping, bool showCat, bool showLevel) {
    timeStamping_ = timeStamping;
    dateStamping_ = dateStamping;
    showCat_ = showCat;
    showLevel_ = showLevel;
    std::cout << getLevelColor(Info);

#ifdef __unix__
    colorOutput_ = true;
#else
    colorOutput_ = false;
#endif
}

std::string ConsoleLog::getLevelColor(LogLevel level) {
    switch (level) {
    case Debug:   return "\033[22;32m";    // green
    case Info:    return "";               // default terminal color
    case Warning: return "\033[01;40;33m"; // orange/yellow on black background
    case Error:   return "\033[22;31m";    // red
    case Fatal:   return "\033[22;34m";    // blue
    default:      return "";
    }
}

void ConsoleLog::logFiltered(const std::string& cat, LogLevel level, const std::string& msg,
                             const std::string& /*extendedInfo*/)
{
    std::string output;
    if (colorOutput_)
        output = getLevelColor(level);

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
    if (colorOutput_)
        output += "\033[00m"; // return to default color (Reset all attributes)

    std::cout << output << '\n';
}

void ConsoleLog::enableColors(bool enable) {
    colorOutput_ = enable;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++

std::string HtmlLog::getLevelString(LogLevel level) {
    switch (level) {
        case Debug:
            return "<td>Debug</TD>";
        case Info:
            return "<td>Info</TD>";
        case Warning:
            return "<td>Warning</TD>";
        case Error:
            return "<td>Error</TD>";
        case Fatal:
            return "<td>Fatal</TD>";
        default:
            return "<td>Unknown</TD>";
    }
}

std::string HtmlLog::getLevelColor(LogLevel level) {
    switch (level) {
        case Debug:
            return "#00CC00";
        case Info:
            return "#FFFFFF";
        case Warning:
            return "#FFFF00";
        case Error:
            return "#FF0000";
        case Fatal:
            return "#0000FF";
        default:
            return "#FFFFFF";
    }
}

void HtmlLog::logFiltered(const std::string &cat, LogLevel level, const std::string &msg,
                          const std::string &extendedInfo)
{
    if (!file_)
        return;

    std::string output = "\t\t\t<tr bgcolor=\"" + getLevelColor(level) + "\">\n";
    if (dateStamping_)
        output += "\t\t\t\t<td>" + getDateString() + "</td>\n";
    if (timeStamping_)
        output += "\t\t\t\t<td>" + getTimeString() + "</td>\n";
    if (showCat_)
        output += "\t\t\t\t<td>" + cat + "</td>\n";
    if (showLevel_)
        output += "\t\t\t\t" + getLevelString(level) + "\n";
    fputs((output +  "\t\t\t\t<td title=\"" + extendedInfo + "\">" + msg + "</td>\n\t\t\t</tr>\n").c_str(), file_);
    fflush(file_);
}

HtmlLog::HtmlLog(const std::string &filename, bool dateStamping, bool timeStamping, bool showCat, bool showLevel) {
    std::string path = (LogMgr.getLogDir().empty() ? "" : LogMgr.getLogDir() + "/");
    file_ = fopen((path + filename).c_str(), "w");

    timeStamping_ = timeStamping;
    dateStamping_ = dateStamping;
    showCat_ = showCat;
    showLevel_ = showLevel;

    if (!file_)
        return;

    std::string output = "<html>\n\t<head>\n\t\t<title>TGT Logfile</title>\n\t</head>\n\t"
        "<body>\n\n\t<table cellpadding=3 cellspacing=0 border=1>\n\t\t"
        "<CAPTION>TGT Logfile</CAPTION>\n\n\t\t<THEAD>\n\t\t\t<TR>\n";
    if (dateStamping_)
        output += "\t\t\t\t<th>Date</th>\n";
    if (timeStamping_)
        output += "\t\t\t\t<th>Time</th>\n";
    if (showCat_)
        output += "\t\t\t\t<th>Category</th>\n";
    if (showLevel_)
        output += "\t\t\t\t<th>Type</th>\n";
    output += "\t\t\t\t<th>Message</th>\n\t\t\t</tr>\n\t\t<tbody>\n";
    fputs(output.c_str(), file_);
    fflush(file_);
}

HtmlLog::~HtmlLog() {
    if (!file_)
        return;
    fputs("\t\t</tbody>\n\t</table>\n\t</body>\n</html>", file_);
    fclose(file_);
}

bool HtmlLog::isOpen() {
    return (file_ != 0);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++

LogManager::LogManager(const std::string& logDir)
    : logDir_(logDir), consoleLog_(0)
{}


LogManager::~LogManager() {
    vector<Log*>::iterator it;
     for (it = logs_.begin(); it != logs_.end(); ++it)
        delete (*it);

    delete consoleLog_;
}

void LogManager::reinit(const std::string& logDir) {
    logDir_ = logDir;
}

void LogManager::log(const std::string &cat, LogLevel level, const std::string &msg,
                     const std::string &extendedInfo)
{
    vector<Log*>::iterator it;
    for (it = logs_.begin(); it != logs_.end(); ++it) {
        if (*it != 0)
            (*it)->log(cat, level, msg, extendedInfo);
    }
    if (consoleLog_)
        consoleLog_->log(cat, level, msg, extendedInfo);
}

void LogManager::addLog(Log* log) {
    ConsoleLog* clog = dynamic_cast<ConsoleLog*>(log);
    if (clog) {
        delete consoleLog_;
        consoleLog_ = clog;
    }
    else
        logs_.push_back(log);
}

void LogManager::removeLog(Log* log) {
    ConsoleLog* clog = dynamic_cast<ConsoleLog*>(log);
    if (clog) {
        delete consoleLog_;
        consoleLog_ = clog;
    } else {
        vector<Log*>::iterator iter = logs_.begin();
        while (iter != logs_.end()) {
            if (*iter == log)
                iter = logs_.erase(iter);
            else
                ++iter;
        }
    }
}

} // namespace tgt

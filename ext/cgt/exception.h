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

#ifndef CGT_EXCEPTION
#define CGT_EXCEPTION

#include "cgt/types.h"

#include <exception>
#include <string>

namespace cgt {

/*
    Note: Use std::bad_alloc for out of memory error
*/

/**
 * Base class for all exceptions.
 */
class CGT_API Exception : public std::exception {
public:
    Exception(const std::string& what = "") : what_(what) {}
    virtual ~Exception() throw() {}

    virtual const char* what() const throw();
protected:
    std::string what_;
};

//------------------------------------------------------------------------------

/**
 * Base class for all file based exceptions.
 */
class CGT_API FileException : public Exception {
public:
    /// @param filename The name of the affected file.
    FileException(const std::string& what = "", const std::string filename = "")
        : Exception(what), filename_(filename) {}
    virtual ~FileException() throw() {}

    /// Returns the name of the affected file.
    const std::string& getFileName() const throw() { return filename_; }

protected:
    std::string getErrorMessage(const std::string& exception, const std::string& what,
                                const std::string& filename) const throw();

    std::string filename_;
};

//------------------------------------------------------------------------------

/**
 * Thrown when a file was not found.
 */
class CGT_API FileNotFoundException : public FileException {
public:
    /// @param filename The name of the file which was not found.
    FileNotFoundException(const std::string& what = "", const std::string& filename = "");
    virtual ~FileNotFoundException() throw() {}
};

//------------------------------------------------------------------------------

/**
 * Thrown when a file couldn't be opened. No proper permessions may be the cause
 * for example.
 */
class CGT_API FileAccessException : public FileException {
public:
    /// @param filename The name of the file which couldn't be opened.
    FileAccessException(const std::string& what = "", const std::string& filename = "");
    virtual ~FileAccessException() throw() {}
};

//------------------------------------------------------------------------------

/**
 * Thrown when a file was tried to load but file/format of the file is
 * corrupted.
 */
class CGT_API CorruptedFileException : public FileException {
public:
    CorruptedFileException(const std::string& what = "", const std::string& filename = "");
    virtual ~CorruptedFileException() throw() {}
};

//------------------------------------------------------------------------------

/**
 * Thrown when a file was tried to load or save in an unsupported format.
 */
class CGT_API UnsupportedFormatException : public FileException {
public:
    /// @param extension The extension which is not supported.
    UnsupportedFormatException(const std::string& extension, const std::string& filename = "");
    virtual ~UnsupportedFormatException() throw() {}
};

//------------------------------------------------------------------------------

/**
 * Thrown when an i/o operation could not be completed successfull. The cause
 * may be a full or damaged disk for instance. Furthermore this Exception is
 * thrown when no special error information is available when dealing with i/o
 * operations. This is the case when using fstream for instance.
 */
class CGT_API IOException : public FileException {
public:
    IOException(const std::string& what = "", const std::string& filename = "");
    virtual ~IOException() throw() {}
};

//------------------------------------------------------------------------------

} // namespace cgt

#endif // CGT_EXCEPTION

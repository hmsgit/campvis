// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#ifndef STRINGUTILS_H__
#define STRINGUTILS_H__

#include "tgt/exception.h"
#include <sstream>
#include <string>
#include <vector>

namespace kisscl {

    /**
     * Collection of various helper methods for strings.
     *
     * \sa std::string
     * 
     * \todo    Test, test, test!
     */
    class StringUtils {
    public:
        /**
         * Converts the string \a str to lowercase.
         * \param str   String to convert.
         * \return Lowercase version of \a str.
         */
        static std::string lowercase(const std::string& str);

        /**
         * Converts the string \a str to uppercase.
         * \param str   String to convert.
         * \return Uppercase version of \a str.
         */
        static std::string uppercase(const std::string& str);

        /**
         * Splits \a str into a vector of strings representing float values.
         * Floats formatted as [-][0-9]*.[0-9]* are considered, all other characters in between are ignored.
         *
         * \param   str     Input string to parse.
         *
         * \note    TODO: The detection algorithm is a litte simplified and will not yield correct results
         *          in every case.
         **/
        static std::vector<std::string> parseFloats(const std::string& str);

        /**
         * Replaces all occurences of \a from in \a str with \a to.
         * \param str   String to perform replacement on.
         * \param from  String to be replaced.
         * \param to    String repace.
         * \return  \a str with all occurences of \a from replaced with \a to.
         */
        static std::string replaceAll(const std::string& str, const std::string& from, const std::string& to);

        /**
         * Splits the string \a str into pieces separated by the delimiters in \a delimiter.
         * \param str       String to split.
         * \param delimiter Set of delimiters.
         * \return  Vector of the split substrings.
         */
        static std::vector<std::string> split(const std::string& str, const std::string& delimiter);

        /**
         * Splits the string \a str into trimmed pieces separated by the delimiters in \a delimiter.
         * Delimiters in quoted strings (\a quotes) will be ignored, double quotes within quoted strings will be 
         * interpreted as literal quotes. Each token will be trimmed.
         *
         * \param str           String to split.
         * \param delimiter     Set of delimiters.
         * \param quotes        Character used for quotes.
         * \param whitespace    Set of whitespace characters which shall be removed during trimming.
         * \return  Vector of the split substrings.
         */
        static std::vector<std::string> splitStringsafe(const std::string& str, const std::string& delimiter, char quotes = '"', const std::string& whitespace = " \t\n\r\0\x0B");

        /**
         * Trims the string \a str.
         * All leading and trailing occurences of the characters in \a whitespace will be removed.
         * 
         * \param str           The string to trim.
         * \param whitespace    Set of whitespace characters which shall be removed at the beginning and the end.
         * \return  The original string without leading and trailing whitespaces.
         */
        static std::string trim(const std::string& str, const std::string& whitespace = " \t\n\r\0\x0B");

        /**
         * Converts the value \a value to a string.
         * \param value     The value to convert, must be compatible with std::stringstream.
         * \return  A string representation of \a value.
         */
        template<class T>
        static std::string toString(const T& value);

        /**
         * Converts the string \a str to its original value.
         * \param str   The string to convert
         * \return  The back-converted value of \a str, type must be compatible with std::stringstream.
         * \throw   tgt::Exception on conversion failure
         */
        template<class T>
        static T fromString(const std::string& str) throw (tgt::Exception);

        /**
         * Joins the substrings in \a tokens together using \a delimiter in between.
         * \param tokens    List of substrings to join.
         * \param delimiter Delimiter which shall be placed between the substrings.
         * \return  A string containing the joined substrings.
         */
        template<typename T>
        static std::string join(const std::vector<T>& tokens, const std::string& delimiter);
    };

// - Template definition --------------------------------------------------------------------------

    template<class T>
    std::string StringUtils::toString(const T& value) {
        std::ostringstream stream;
        stream << value;
        return stream.str();
    }

    template<class T>
    T StringUtils::fromString(const std::string& str) throw (tgt::Exception) {
        T toReturn;
        std::istringstream stream;
        stream.str(str);
        if (!(stream >> toReturn))
            throw tgt::Exception("Failed to convert string '" + str + "'");
        return toReturn;
    }

    template<typename T>
    std::string StringUtils::join(const std::vector<T>& tokens, const std::string& delimiter) {
        if (tokens.empty())
            return "";
        std::stringstream s;
        s << tokens[0];
        for (std::vector<T>::size_type i = 1; i < tokens.size(); ++i)
            s << delimiter << tokens[i];
        return s.str();
    }

}

#endif // STRINGUTILS_H__
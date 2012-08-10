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

#include "stringutils.h"

#include <algorithm>


namespace TUMVis {

    std::string StringUtils::lowercase(const std::string& str) {
        std::string toReturn(str);
        std::transform(toReturn.begin(), toReturn.end(), toReturn.begin(), ::tolower);
        return toReturn;
    }

    std::string StringUtils::uppercase(const std::string& str) {
        std::string toReturn(str);
        std::transform(toReturn.begin(), toReturn.end(), toReturn.begin(), ::toupper);
        return toReturn;
    }

    std::vector<std::string> StringUtils::parseFloats(const std::string& str) {
        static const std::string floatCharacters("0123456789.-");
        std::vector<std::string> toReturn;
        size_t strpos = 0;
        size_t endpos = 0;

        // we just started or just finished parsing an entry, check if finished and skip to beginning of next entry
        while ((endpos != std::string::npos) && (strpos = str.find_first_of(floatCharacters, strpos)) != std::string::npos) {
            // strpos currently points to the beginning of a float, now find its end
            endpos = str.find_first_not_of(floatCharacters, strpos+1);

            // extract float
            std::string token = str.substr(strpos, endpos - strpos);

            // sanity checks
            size_t signPos = token.rfind('-');
            if (signPos == 0 || signPos == std::string::npos) { // sign only allowed at beginning
                if (token.find('.') == token.rfind('.')) {      // only one . allowed
                    toReturn.push_back(token);
                }
            }
            strpos = endpos + 1;
        }
        return toReturn;
    }

    std::string StringUtils::replaceAll(const std::string& str, const std::string& from, const std::string& to) {
        std::string toReturn(str);
        std::string::size_type strpos = 0;
        std::string::size_type foundpos;
        while((foundpos = toReturn.find(from, strpos)) != std::string::npos) {
            toReturn.replace(foundpos, from.size(), to);
            strpos = foundpos + to.size();
        }
        return toReturn;
    }

    std::vector<std::string> StringUtils::split(const std::string& line, const std::string& delimiter) {
        std::vector<std::string> toReturn;
        std::string::size_type linepos = 0;
        std::string::size_type endpos = 0;

        // we are at the beginning of an entry, skip whitespaces and check if not already reached end of line
        while (endpos != std::string::npos) {
            endpos = line.find_first_of(delimiter, linepos);
            toReturn.push_back(line.substr(linepos, endpos - linepos));
            linepos = endpos + 1;
        }
        return toReturn;
    }

    std::vector<std::string> StringUtils::splitStringsafe(const std::string& str, const std::string& delimiter, char quotes /*= '"'*/, const std::string& whitespace /*= " \t"*/) {
        std::vector<std::string> toReturn;
        std::string::size_type strpos = 0;
        std::string::size_type endpos = 0;

        // we are at the beginning of an entry, skip whitespaces and check if not already reached end of str
        while ((endpos != std::string::npos) && (strpos = str.find_first_not_of(whitespace, strpos)) != std::string::npos) {
            // now strpos points to the first non blank character, here starts the entry
            // check whether there are quotes
            if (str[strpos] == quotes) {
                // find position of closing quotes
                endpos = str.find_first_of('"', strpos + 1);

                std::string toPush = str.substr(strpos + 1 , endpos - strpos - 1);
                // ensure we haven't found double quotes ("") which shall be resolved to one double quote in resulting string
                while ((endpos != std::string::npos) && (endpos + 1 < str.length()) && (str[endpos + 1] == '"')) {
                    strpos = endpos + 1;
                    endpos = str.find_first_of('"', endpos + 2);
                    toPush.append(str.substr(strpos, endpos - strpos));
                }

                // push string in quotes onto toReturn
                toReturn.push_back(StringUtils::trim(toPush, whitespace));

                // ignore everything until next delimiter
                endpos = str.find_first_of(delimiter, endpos);
            }
            // ok, this entry is not in quotes - just push everything until next delimiter onto toReturn
            else {
                endpos = str.find_first_of(delimiter, strpos);
                toReturn.push_back(StringUtils::trim(str.substr(strpos, endpos - strpos), whitespace));
            }
            strpos = endpos + 1;
        }
        return toReturn;
    }

    std::string StringUtils::trim(const std::string& str, const std::string& whitespace /*= " \t"*/) {
        std::string::size_type first = str.find_first_not_of(whitespace);
        if(first == std::string::npos) {
            return "";
        }
        else {
            std::string::size_type last = str.find_last_not_of(whitespace);
            return str.substr(first, last - first + 1);
        } 
    }
}
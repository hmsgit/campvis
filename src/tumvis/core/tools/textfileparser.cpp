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

#include "textfileparser.h"

#include <sstream>


namespace TUMVis {

    TextFileParser::TextFileParser(const std::string& url, bool caseSensitiveKeys, const std::string& delimiters, const std::string& whitespace /*= " \t"*/) 
        : _url(url)
        , _caseSensitiveKeys(caseSensitiveKeys)
        , _delimiters(delimiters)
        , _whitespace(whitespace)
    {
    }

    bool TextFileParser::hasKey(const std::string& key) const {
        return (_tokens.find(key) != _tokens.end());
    }

    const std::string& TextFileParser::getString(const std::string& key) const throw (tgt::CorruptedFileException) {
        std::map<std::string, std::string>::const_iterator it = (_caseSensitiveKeys ? _tokens.find(key) : _tokens.find(StringUtils::lowercase(key)));
        if (it == _tokens.end()) {
            throw tgt::CorruptedFileException("No token with key " + key + " found.", _url);
        }
        else {
            return it->second;
        }
    }

    bool TextFileParser::getBool(const std::string& key) const throw (tgt::CorruptedFileException) {
        std::string lc = StringUtils::lowercase(getString(key));

        if ((lc == "0") || (lc == "false"))
            return false;
        else if ((lc == "1") || (lc == "true"))
            return true;
        else
            throw tgt::CorruptedFileException("Error parsing key " + key + " to bool.", _url);
    }

    int TextFileParser::getInt(const std::string& key) const throw (tgt::CorruptedFileException) {
        std::string str = getString(key);
        try {
            return StringUtils::fromString<int>(str);
        }
        catch (std::exception& e) {
            throw tgt::CorruptedFileException("Error parsing key " + key + " to int: " + e.what(), _url);
        }        
    }

    tgt::ivec2 TextFileParser::getIvec2(const std::string& key) const  throw (tgt::CorruptedFileException){
        std::string str = getString(key);
        try {
            return StringUtils::fromString<tgt::ivec2>(str);
        }
        catch (std::exception& e) {
            throw tgt::CorruptedFileException("Error parsing key " + key + " to ivec2: " + e.what(), _url);
        }        
    }

    tgt::ivec3 TextFileParser::getIvec3(const std::string& key) const  throw (tgt::CorruptedFileException){
        std::string str = getString(key);
        try {
            return StringUtils::fromString<tgt::ivec3>(str);
        }
        catch (std::exception& e) {
            throw tgt::CorruptedFileException("Error parsing key " + key + " to ivec3: " + e.what(), _url);
        }        
    }

    tgt::ivec4 TextFileParser::getIvec4(const std::string& key) const  throw (tgt::CorruptedFileException){
        std::string str = getString(key);
        try {
            return StringUtils::fromString<tgt::ivec4>(str);
        }
        catch (std::exception& e) {
            throw tgt::CorruptedFileException("Error parsing key " + key + " to ivec4: " + e.what(), _url);
        }        
    }

    size_t TextFileParser::getSizeT(const std::string& key) const throw (tgt::CorruptedFileException) {
        std::string str = getString(key);
        try {
            return StringUtils::fromString<size_t>(str);
        }
        catch (std::exception& e) {
            throw tgt::CorruptedFileException("Error parsing key " + key + " to size_t: " + e.what(), _url);
        }        
    }

    tgt::svec2 TextFileParser::getSvec2(const std::string& key) const  throw (tgt::CorruptedFileException){
        std::string str = getString(key);
        try {
            return StringUtils::fromString<tgt::svec2>(str);
        }
        catch (std::exception& e) {
            throw tgt::CorruptedFileException("Error parsing key " + key + " to svec2: " + e.what(), _url);
        }        
    }

    tgt::svec3 TextFileParser::getSvec3(const std::string& key) const  throw (tgt::CorruptedFileException){
        std::string str = getString(key);
        try {
            return StringUtils::fromString<tgt::svec3>(str);
        }
        catch (std::exception& e) {
            throw tgt::CorruptedFileException("Error parsing key " + key + " to svec3: " + e.what(), _url);
        }        
    }

    tgt::svec4 TextFileParser::getSvec4(const std::string& key) const  throw (tgt::CorruptedFileException){
        std::string str = getString(key);
        try {
            return StringUtils::fromString<tgt::svec4>(str);
        }
        catch (std::exception& e) {
            throw tgt::CorruptedFileException("Error parsing key " + key + " to svec4: " + e.what(), _url);
        }        
    }

    float TextFileParser::getFloat(const std::string& key) const throw (tgt::CorruptedFileException) {
        std::string str = getString(key);
        try {
            return StringUtils::fromString<float>(str);
        }
        catch (std::exception& e) {
            throw tgt::CorruptedFileException("Error parsing key " + key + " to float: " + e.what(), _url);
        }        
    }

    tgt::vec2 TextFileParser::getVec2(const std::string& key) const throw (tgt::CorruptedFileException) {
        std::string str = getString(key);
        try {
            return StringUtils::fromString<tgt::vec2>(str);
        }
        catch (std::exception& e) {
            throw tgt::CorruptedFileException("Error parsing key " + key + " to vec2: " + e.what(), _url);
        }        
    }

    tgt::vec3 TextFileParser::getVec3(const std::string& key) const throw (tgt::CorruptedFileException) {
        std::string str = getString(key);
        try {
            return StringUtils::fromString<tgt::vec3>(str);
        }
        catch (std::exception& e) {
            throw tgt::CorruptedFileException("Error parsing key " + key + " to vec3: " + e.what(), _url);
        }        
    }

    tgt::vec4 TextFileParser::getVec4(const std::string& key) const throw (tgt::CorruptedFileException) {
        std::string str = getString(key);
        try {
            return StringUtils::fromString<tgt::vec4>(str);
        }
        catch (std::exception& e) {
            throw tgt::CorruptedFileException("Error parsing key " + key + " to vec4: " + e.what(), _url);
        }        
    }
}
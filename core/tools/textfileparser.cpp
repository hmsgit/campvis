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

#include "textfileparser.h"

#include <sstream>


namespace campvis {

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

// ================================================================================================

    template<>
    std::vector<std::string> TextFileParser::readAndParseItems<TextFileParser::ItemSeparatorLines>() const throw (tgt::FileException, tgt::CorruptedFileException) {
        tgt::File* file = FileSys.open(_url);
        if (!file || !file->isOpen())
            throw tgt::FileException("Could not open file " + _url + " for reading.", _url);

        std::vector<std::string> lines;
        while (!file->eof()) {
            lines.push_back(file->getLine());
        }
        file->close();
        delete file;
        return lines;
    }

}
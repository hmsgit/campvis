// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
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

#include <stack>
#include <sstream>

namespace campvis {

    TextFileParser::TextFileParser(std::istream& stream, bool caseSensitiveKeys, const std::string& delimiters, const std::string& whitespace /*= " \t"*/) 
        : _stream(stream)
        , _caseSensitiveKeys(caseSensitiveKeys)
        , _delimiters(delimiters)
        , _whitespace(whitespace)
        , _rootGroup(nullptr)
    {
    }

    const TextFileParser::TokenGroup* TextFileParser::getRootGroup() const {
        return _rootGroup;
    }

    TextFileParser::~TextFileParser() {
        delete _rootGroup;
    }

    bool TextFileParser::TokenGroup::hasKey(const std::string& key) const {
        auto it = (_caseSensitiveKeys ? _tokens.find(key) : _tokens.find(StringUtils::lowercase(key)));
        return (it != _tokens.end());
    }

    const std::string& TextFileParser::TokenGroup::getString(const std::string& key) const throw (cgt::Exception) {
        std::map<std::string, std::string>::const_iterator it = (_caseSensitiveKeys ? _tokens.find(key) : _tokens.find(StringUtils::lowercase(key)));
        if (it == _tokens.end()) {
            throw cgt::Exception("No token with key " + key + " found.");
        }
        else {
            return it->second;
        }
    }

    bool TextFileParser::TokenGroup::getBool(const std::string& key) const throw (cgt::Exception) {
        std::string lc = StringUtils::lowercase(getString(key));

        if ((lc == "0") || (lc == "false"))
            return false;
        else if ((lc == "1") || (lc == "true"))
            return true;
        else
            throw cgt::Exception("Error parsing key " + key + " to bool.");
    }

    int TextFileParser::TokenGroup::getInt(const std::string& key) const throw (cgt::Exception) {
        std::string str = getString(key);
        try {
            return StringUtils::fromString<int>(str);
        }
        catch (std::exception& e) {
            throw cgt::Exception("Error parsing key " + key + " to int: " + e.what());
        }        
    }

    cgt::ivec2 TextFileParser::TokenGroup::getIvec2(const std::string& key) const  throw (cgt::Exception){
        std::string str = getString(key);
        try {
            return StringUtils::fromString<cgt::ivec2>(str);
        }
        catch (std::exception& e) {
            throw cgt::Exception("Error parsing key " + key + " to ivec2: " + e.what());
        }        
    }

    cgt::ivec3 TextFileParser::TokenGroup::getIvec3(const std::string& key) const  throw (cgt::Exception){
        std::string str = getString(key);
        try {
            return StringUtils::fromString<cgt::ivec3>(str);
        }
        catch (std::exception& e) {
            throw cgt::Exception("Error parsing key " + key + " to ivec3: " + e.what());
        }        
    }

    cgt::ivec4 TextFileParser::TokenGroup::getIvec4(const std::string& key) const  throw (cgt::Exception){
        std::string str = getString(key);
        try {
            return StringUtils::fromString<cgt::ivec4>(str);
        }
        catch (std::exception& e) {
            throw cgt::Exception("Error parsing key " + key + " to ivec4: " + e.what());
        }        
    }

    size_t TextFileParser::TokenGroup::getSizeT(const std::string& key) const throw (cgt::Exception) {
        std::string str = getString(key);
        try {
            return StringUtils::fromString<size_t>(str);
        }
        catch (std::exception& e) {
            throw cgt::Exception("Error parsing key " + key + " to size_t: " + e.what());
        }        
    }

    cgt::svec2 TextFileParser::TokenGroup::getSvec2(const std::string& key) const  throw (cgt::Exception){
        std::string str = getString(key);
        try {
            return StringUtils::fromString<cgt::svec2>(str);
        }
        catch (std::exception& e) {
            throw cgt::Exception("Error parsing key " + key + " to svec2: " + e.what());
        }        
    }

    cgt::svec3 TextFileParser::TokenGroup::getSvec3(const std::string& key) const  throw (cgt::Exception){
        std::string str = getString(key);
        try {
            return StringUtils::fromString<cgt::svec3>(str);
        }
        catch (std::exception& e) {
            throw cgt::Exception("Error parsing key " + key + " to svec3: " + e.what());
        }        
    }

    cgt::svec4 TextFileParser::TokenGroup::getSvec4(const std::string& key) const  throw (cgt::Exception){
        std::string str = getString(key);
        try {
            return StringUtils::fromString<cgt::svec4>(str);
        }
        catch (std::exception& e) {
            throw cgt::Exception("Error parsing key " + key + " to svec4: " + e.what());
        }        
    }

    float TextFileParser::TokenGroup::getFloat(const std::string& key) const throw (cgt::Exception) {
        std::string str = getString(key);
        try {
            return StringUtils::fromString<float>(str);
        }
        catch (std::exception& e) {
            throw cgt::Exception("Error parsing key " + key + " to float: " + e.what());
        }        
    }

    cgt::vec2 TextFileParser::TokenGroup::getVec2(const std::string& key) const throw (cgt::Exception) {
        std::string str = getString(key);
        try {
            return StringUtils::fromString<cgt::vec2>(str);
        }
        catch (std::exception& e) {
            throw cgt::Exception("Error parsing key " + key + " to vec2: " + e.what());
        }        
    }

    cgt::vec3 TextFileParser::TokenGroup::getVec3(const std::string& key) const throw (cgt::Exception) {
        std::string str = getString(key);
        try {
            return StringUtils::fromString<cgt::vec3>(str);
        }
        catch (std::exception& e) {
            throw cgt::Exception("Error parsing key " + key + " to vec3: " + e.what());
        }        
    }

    cgt::vec4 TextFileParser::TokenGroup::getVec4(const std::string& key) const throw (cgt::Exception) {
        std::string str = getString(key);
        try {
            return StringUtils::fromString<cgt::vec4>(str);
        }
        catch (std::exception& e) {
            throw cgt::Exception("Error parsing key " + key + " to vec4: " + e.what());
        }        
    }

    TextFileParser::TokenGroup::~TokenGroup() {
        for (auto it = _tokenGroups.begin(); it != _tokenGroups.end(); ++it)
            delete it->second;
    }

    TextFileParser::TokenGroup* TextFileParser::ItemSeparatorLines::operator()(std::istream& stream, bool caseSensitiveKeys, const std::string& delimiters, const std::string& whitespace) {
        TokenGroup* rootNode = new TokenGroup();
        std::stack<TokenGroup*> groupHierarchy;
        groupHierarchy.push(rootNode);
        std::string currentLine;

        while (stream.good() && !stream.eof()) {
            std::getline(stream, currentLine);
            currentLine = StringUtils::trim(currentLine);
            
            if (currentLine == "")
                continue;
            if (currentLine.substr(currentLine.length() - 1, 1) == "{") {
                std::string groupName = StringUtils::trim(currentLine.substr(0, currentLine.length() - 1));

                TokenGroup* tg = new TokenGroup();
                groupHierarchy.top()->_tokenGroups.insert(std::make_pair(groupName, tg));
                groupHierarchy.push(tg);
            }
            else if (currentLine == "}") {
                groupHierarchy.pop();
            }
            else {
                std::vector<std::string> tokens = StringUtils::splitStringsafe(currentLine, delimiters, '"');
                if (tokens.size() == 2) {
                    if (caseSensitiveKeys)
                        groupHierarchy.top()->_tokens.insert(std::make_pair(StringUtils::trim(tokens[0], whitespace), StringUtils::trim(tokens[1], whitespace)));
                    else
                        groupHierarchy.top()->_tokens.insert(std::make_pair(StringUtils::lowercase(StringUtils::trim(tokens[0], whitespace)), StringUtils::trim(tokens[1], whitespace)));
                }
                else {
                    throw cgt::Exception("Error parsing item '" + currentLine + "': expected single delimiter.");
                }
            }
        }

        return rootNode;
    }

}
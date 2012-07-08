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

    bool TextFileParser::hasToken(const std::string& key) const {
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
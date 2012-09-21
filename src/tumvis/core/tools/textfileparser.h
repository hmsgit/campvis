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

#ifndef TEXTFILEPARSER_H__
#define TEXTFILEPARSER_H__

#include "tgt/exception.h"
#include "tgt/filesystem.h"
#include "tgt/vector.h"

#include "core/tools/stringutils.h"

#include <map>
#include <functional>
#include <string>
#include <vector>

namespace TUMVis {

    /**
     * Helper class for parsing simple text files containing key-value pairs.
     * The file will be read in text mode and split into single key-value items using the template argument
     * functor of parse(). Finally, each item will be split into key and value using the given delimiters.
     *
     * \todo    Better vector support (e.g. "x,y,z" format)
     *          Test and debug o_O
     */
    class TextFileParser {
    public:
        /**
         * Item separator for letting each line in the file result in one key-value pair item.
         * \note    The behaviour is not implemented as functor but by template specialization!
         */
        struct ItemSeparatorLines {
            // no operator() to implement due to template specialization
        };

        /**
         * Creates a new text file parser with the given settings.
         * \param url               URL of file
         * \param caseSensitiveKeys Flag whether keys are case-sensitive or not
         * \param delimiters        Set of delimiters for separating key-value pair
         * \param whitespace        Set of characters identifying whitespace
         */
        TextFileParser(const std::string& url, bool caseSensitiveKeys, const std::string& delimiters, const std::string& whitespace = " \t\n\r");


        /**
         * Performs the parsing of the text file into key-value pairs.
         * The file will be read in text mode and split into single items using the template argument
         * functor \a T. Then, each item will be split into key-value pairs.
         * 
         * \sa TextFileParser::ItemSeparatorLines
         * \tparam  T   Functor for splitting the file into items, must implement std::unary_function<std::string, std::vector<std::string> >
         * \throw   tgt::FileException if file not found/not readable, tgt::CorruptedFileException on parsing errors during key-value pair parsing.
         */
        template<class T>
        void parse() throw (tgt::FileException, tgt::CorruptedFileException);

        /**
         * Checks whether there exists a token with the given key \a key.
         * \param key   The key to search for.
         * \return  True if a key-value pair with the given key is existent, otherwise false.
         */
        bool hasKey(const std::string& key) const;

        /**
         * Returns the value to the given key \a key.
         * \param key   The key to search for.
         * \return      The corresponding value to the given key.
         * \throw       tgt::CorruptedFileException if no such key existent.
         */
        const std::string& getString(const std::string& key) const throw (tgt::CorruptedFileException);

        /**
         * Returns the bool representation of the value for the given key \a key.
         * \param key   The key to search for.
         * \return      Boolean representation for the corresponding value to the given key.
         * \throw       tgt::CorruptedFileException if no such key existent or conversion failed.
         */
        bool getBool(const std::string& key) const throw (tgt::CorruptedFileException);

        /**
         * Returns the integer representation of the value for the given key \a key.
         * \param key   The key to search for.
         * \return      Integer representation for the corresponding value to the given key.
         * \throw       tgt::CorruptedFileException if no such key existent or conversion failed.
         */
        int getInt(const std::string& key) const throw (tgt::CorruptedFileException);

        /**
         * Returns the ivec2 representation of the value for the given key \a key.
         * \param key   The key to search for.
         * \return      ivec2 representation for the corresponding value to the given key.
         * \throw       tgt::CorruptedFileException if no such key existent or conversion failed.
         */
        tgt::ivec2 getIvec2(const std::string& key) const throw (tgt::CorruptedFileException);

        /**
         * Returns the ivec3 representation of the value for the given key \a key.
         * \param key   The key to search for.
         * \return      ivec3 representation for the corresponding value to the given key.
         * \throw       tgt::CorruptedFileException if no such key existent or conversion failed.
         */
        tgt::ivec3 getIvec3(const std::string& key) const throw (tgt::CorruptedFileException);

        /**
         * Returns the ivec4 representation of the value for the given key \a key.
         * \param key   The key to search for.
         * \return      ivec4 representation for the corresponding value to the given key.
         * \throw       tgt::CorruptedFileException if no such key existent or conversion failed.
         */
        tgt::ivec4 getIvec4(const std::string& key) const throw (tgt::CorruptedFileException);


        /**
         * Returns the size_t representation of the value for the given key \a key.
         * \param key   The key to search for.
         * \return      size_t representation for the corresponding value to the given key.
         * \throw       tgt::CorruptedFileException if no such key existent or conversion failed.
         */
        size_t getSizeT(const std::string& key) const throw (tgt::CorruptedFileException);

        /**
         * Returns the svec2 representation of the value for the given key \a key.
         * \param key   The key to search for.
         * \return      svec2 representation for the corresponding value to the given key.
         * \throw       tgt::CorruptedFileException if no such key existent or conversion failed.
         */
        tgt::svec2 getSvec2(const std::string& key) const throw (tgt::CorruptedFileException);

        /**
         * Returns the svec3 representation of the value for the given key \a key.
         * \param key   The key to search for.
         * \return      svec3 representation for the corresponding value to the given key.
         * \throw       tgt::CorruptedFileException if no such key existent or conversion failed.
         */
        tgt::svec3 getSvec3(const std::string& key) const throw (tgt::CorruptedFileException);

        /**
         * Returns the svec4 representation of the value for the given key \a key.
         * \param key   The key to search for.
         * \return      svec4 representation for the corresponding value to the given key.
         * \throw       tgt::CorruptedFileException if no such key existent or conversion failed.
         */
        tgt::svec4 getSvec4(const std::string& key) const throw (tgt::CorruptedFileException);


        /**
         * Returns the float representation of the value for the given key \a key.
         * \param key   The key to search for.
         * \return      Float representation for the corresponding value to the given key.
         * \throw       tgt::CorruptedFileException if no such key existent or conversion failed.
         */
        float getFloat(const std::string& key) const throw (tgt::CorruptedFileException);

        /**
         * Returns the vec2 representation of the value for the given key \a key.
         * \param key   The key to search for.
         * \return      vec2 representation for the corresponding value to the given key.
         * \throw       tgt::CorruptedFileException if no such key existent or conversion failed.
         */
        tgt::vec2 getVec2(const std::string& key) const throw (tgt::CorruptedFileException);

        /**
         * Returns the vec3 representation of the value for the given key \a key.
         * \param key   The key to search for.
         * \return      vec3 representation for the corresponding value to the given key.
         * \throw       tgt::CorruptedFileException if no such key existent or conversion failed.
         */
        tgt::vec3 getVec3(const std::string& key) const throw (tgt::CorruptedFileException);

        /**
         * Returns the vec4 representation of the value for the given key \a key.
         * \param key   The key to search for.
         * \return      vec4 representation for the corresponding value to the given key.
         * \throw       tgt::CorruptedFileException if no such key existent or conversion failed.
         */
        tgt::vec4 getVec4(const std::string& key) const throw (tgt::CorruptedFileException);


    protected:
        /**
         * Loads the text file and parses it into items.
         * Item parsing is done using the Functor of the template parameter.
         * 
         * \sa TextFileParser::ItemSeparatorLines
         * \tparam  T   Functor for splitting the file into items, must implement std::unary_function<std::string, std::vector<std::string> >
         * \throw   tgt::FileException if file not found/not readable, tgt::CorruptedFileException on parsing errors during key-value pair parsing.
         */
        template<class T>
        inline std::vector<std::string> readAndParseItems() const throw (tgt::FileException, tgt::CorruptedFileException) ;

        std::string _url;                               ///< URL of file
        bool _caseSensitiveKeys;                        ///< Flag whether keys are case-sensitive or not
        std::string _delimiters;                        ///< Set of delimiters for separating key-value pair
        std::string _whitespace;                        ///< Set of characters identifying whitespace

        std::map<std::string, std::string> _tokens;     ///< map of key-value pairs
    };

// - Template definitions -------------------------------------------------------------------------

    template<typename T>
    void TextFileParser::parse() throw (tgt::FileException, tgt::CorruptedFileException) {
        _tokens.clear();
        std::vector<std::string> items = readAndParseItems<T>();

        for (std::vector<std::string>::const_iterator it = items.begin(); it != items.end(); ++it) {
            std::vector<std::string> tokens = StringUtils::split(*it, _delimiters);
            if (tokens.size() == 2) {
                if (_caseSensitiveKeys)
                    _tokens.insert(std::make_pair(StringUtils::trim(tokens[0], _whitespace), StringUtils::trim(tokens[1], _whitespace)));
                else
                    _tokens.insert(std::make_pair(StringUtils::lowercase(StringUtils::trim(tokens[0], _whitespace)), StringUtils::trim(tokens[1], _whitespace)));
            }
            else {
                throw tgt::CorruptedFileException("Error parsing item '" + *it + "': expected delimiter.", _url);
            }
        }
    }

    template<typename T>
    inline std::vector<std::string> TUMVis::TextFileParser::readAndParseItems() const throw (tgt::FileException, tgt::CorruptedFileException) {
        tgt::File* file = FileSys.open(_url);
        if (!file || !file->isOpen())
            throw tgt::FileException("Could not open file " + _url + " for reading.", _url);

        // somewhat cumbersome but it works:
        // read file line-based, glue them back together and run item spitter.
        std::vector<std::string> lines;
        while (!file->eof()) {
            lines.push_back(file->getLine('\n'));
        }
        std::string data = StringUtils::join(lines, "\n");
        file->close();
        delete file;

        T itemSplitter;
        return itemSplitter(data);
    }

// - Template specializations ---------------------------------------------------------------------

    /// Template specialization for avoiding redundant split-join-split of lines.
    template<>
    inline std::vector<std::string> TUMVis::TextFileParser::readAndParseItems<TextFileParser::ItemSeparatorLines>() const throw (tgt::FileException, tgt::CorruptedFileException) {
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

#endif // TEXTFILEPARSER_H__

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

#ifndef TEXTFILEPARSER_H__
#define TEXTFILEPARSER_H__

#include "cgt/exception.h"
#include "cgt/filesystem.h"
#include "cgt/vector.h"

#include "core/coreapi.h"
#include "core/tools/stringutils.h"

#include <map>
#include <functional>
#include <istream>
#include <string>
#include <vector>

namespace campvis {

    /**
     * Helper class for parsing simple text files containing key-value pairs.
     * The file will be read in text mode and split into single key-value items using the template argument
     * functor of parse(). Finally, each item will be split into key and value using the given delimiters.
     *
     * \todo    Better vector support (e.g. "x,y,z" format)
     *          Test and debug o_O
     */
    class CAMPVIS_CORE_API TextFileParser {
    public:
        struct CAMPVIS_CORE_API TokenGroup {
        public:
            ~TokenGroup();

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
             * \throw       cgt::Exception if no such key existent.
             */
            const std::string& getString(const std::string& key) const throw (cgt::Exception);

            /**
             * Returns the bool representation of the value for the given key \a key.
             * \param key   The key to search for.
             * \return      Boolean representation for the corresponding value to the given key.
             * \throw       cgt::Exception if no such key existent or conversion failed.
             */
            bool getBool(const std::string& key) const throw (cgt::Exception);

            /**
             * Returns the integer representation of the value for the given key \a key.
             * \param key   The key to search for.
             * \return      Integer representation for the corresponding value to the given key.
             * \throw       cgt::Exception if no such key existent or conversion failed.
             */
            int getInt(const std::string& key) const throw (cgt::Exception);

            /**
             * Returns the ivec2 representation of the value for the given key \a key.
             * \param key   The key to search for.
             * \return      ivec2 representation for the corresponding value to the given key.
             * \throw       cgt::Exception if no such key existent or conversion failed.
             */
            cgt::ivec2 getIvec2(const std::string& key) const throw (cgt::Exception);

            /**
             * Returns the ivec3 representation of the value for the given key \a key.
             * \param key   The key to search for.
             * \return      ivec3 representation for the corresponding value to the given key.
             * \throw       cgt::Exception if no such key existent or conversion failed.
             */
            cgt::ivec3 getIvec3(const std::string& key) const throw (cgt::Exception);

            /**
             * Returns the ivec4 representation of the value for the given key \a key.
             * \param key   The key to search for.
             * \return      ivec4 representation for the corresponding value to the given key.
             * \throw       cgt::Exception if no such key existent or conversion failed.
             */
            cgt::ivec4 getIvec4(const std::string& key) const throw (cgt::Exception);


            /**
             * Returns the size_t representation of the value for the given key \a key.
             * \param key   The key to search for.
             * \return      size_t representation for the corresponding value to the given key.
             * \throw       cgt::Exception if no such key existent or conversion failed.
             */
            size_t getSizeT(const std::string& key) const throw (cgt::Exception);

            /**
             * Returns the svec2 representation of the value for the given key \a key.
             * \param key   The key to search for.
             * \return      svec2 representation for the corresponding value to the given key.
             * \throw       cgt::Exception if no such key existent or conversion failed.
             */
            cgt::svec2 getSvec2(const std::string& key) const throw (cgt::Exception);

            /**
             * Returns the svec3 representation of the value for the given key \a key.
             * \param key   The key to search for.
             * \return      svec3 representation for the corresponding value to the given key.
             * \throw       cgt::Exception if no such key existent or conversion failed.
             */
            cgt::svec3 getSvec3(const std::string& key) const throw (cgt::Exception);

            /**
             * Returns the svec4 representation of the value for the given key \a key.
             * \param key   The key to search for.
             * \return      svec4 representation for the corresponding value to the given key.
             * \throw       cgt::Exception if no such key existent or conversion failed.
             */
            cgt::svec4 getSvec4(const std::string& key) const throw (cgt::Exception);


            /**
             * Returns the float representation of the value for the given key \a key.
             * \param key   The key to search for.
             * \return      Float representation for the corresponding value to the given key.
             * \throw       cgt::Exception if no such key existent or conversion failed.
             */
            float getFloat(const std::string& key) const throw (cgt::Exception);

            /**
             * Returns the vec2 representation of the value for the given key \a key.
             * \param key   The key to search for.
             * \return      vec2 representation for the corresponding value to the given key.
             * \throw       cgt::Exception if no such key existent or conversion failed.
             */
            cgt::vec2 getVec2(const std::string& key) const throw (cgt::Exception);

            /**
             * Returns the vec3 representation of the value for the given key \a key.
             * \param key   The key to search for.
             * \return      vec3 representation for the corresponding value to the given key.
             * \throw       cgt::Exception if no such key existent or conversion failed.
             */
            cgt::vec3 getVec3(const std::string& key) const throw (cgt::Exception);

            /**
             * Returns the vec4 representation of the value for the given key \a key.
             * \param key   The key to search for.
             * \return      vec4 representation for the corresponding value to the given key.
             * \throw       cgt::Exception if no such key existent or conversion failed.
             */
            cgt::vec4 getVec4(const std::string& key) const throw (cgt::Exception);

            std::multimap<std::string, TokenGroup*> _tokenGroups;   ///< multimap of token groups
            std::map<std::string, std::string> _tokens;             ///< map of key-value pairs

        private:
            bool _caseSensitiveKeys;                                ///< Flag whether keys are case-sensitive or not
        };


        /**
         * Item separator for letting each line in the file result in one key-value pair item.
         * \note    The behaviour is not implemented as functor but by template specialization!
         */
        struct CAMPVIS_CORE_API ItemSeparatorLines {
            TokenGroup* operator()(std::istream& stream, bool caseSensitiveKeys, const std::string& delimiters, const std::string& whitespace);
        };

        /**
         * Creates a new text file parser with the given settings.
         * \param stream            Input stream to read from
         * \param caseSensitiveKeys Flag whether keys are case-sensitive or not
         * \param delimiters        Set of delimiters for separating key-value pair
         * \param whitespace        Set of characters identifying whitespace
         */
        TextFileParser(std::istream& stream, bool caseSensitiveKeys, const std::string& delimiters, const std::string& whitespace = " \t\n\r");

        ~TextFileParser();

        /**
         * Performs the parsing of the text file into key-value pairs.
         * The file will be read in text mode and split into single items using the template argument
         * functor \a T. Then, each item will be split into key-value pairs.
         * 
         * \sa TextFileParser::ItemSeparatorLines
         * \tparam  T   Functor for splitting the file into items, must implement std::unary_function<std::string, std::vector<std::string> >
         * \throw   cgt::FileException if file not found/not readable, cgt::Exception on parsing errors during key-value pair parsing.
         */
        template<class T>
        void parse() throw (cgt::FileException, cgt::Exception);


        const TokenGroup* getRootGroup() const;

    protected:
        std::istream& _stream;                          ///< Input stream to read from
        bool _caseSensitiveKeys;                        ///< Flag whether keys are case-sensitive or not
        std::string _delimiters;                        ///< Set of delimiters for separating key-value pair
        std::string _whitespace;                        ///< Set of characters identifying whitespace

        TokenGroup* _rootGroup;                         ///< root token group
    };
    
// - Template definitions -------------------------------------------------------------------------

    template<typename T>
    void TextFileParser::parse() throw (cgt::FileException, cgt::Exception) {
        delete _rootGroup;

        T parser;
        _rootGroup = parser(_stream, _caseSensitiveKeys, _delimiters, _whitespace);
    }

}

#endif // TEXTFILEPARSER_H__

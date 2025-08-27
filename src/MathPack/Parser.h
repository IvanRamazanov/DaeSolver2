/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#ifndef MATHPACK_PARSER_H
#define MATHPACK_PARSER_H

#include <iostream>
#include <unordered_map>
#include <variant>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <iomanip>
#include <filesystem>
#include <fstream>
#include <concepts>
#include <type_traits>

using namespace std;
//using attr_type = variant<int, bool, string>;

namespace XmlParser{
    /**
     *
     * @author Ivan Ramazanov
     */

    /**
     * Custom XML parser
     */
    class XmlElement{
        public:
            string name;
            unordered_map<string, string> attributes;
            string text; // tail?

            XmlElement(string const& name);

            ~XmlElement();

            vector<string> toStrings(bool pretty_print=false);

            void append(XmlElement* child);

            vector<XmlElement*> findAll(string const& tag);

            /**
             * find first match
             */
            XmlElement* find(string const& tag);

        private:
            enum block_types {comment, instruction, regular, error};

            vector<XmlElement*> children;

            static XmlElement* parseElement(string::iterator &text, const string::iterator &until);

            /**
             * Skips <!-- -->
             */
            static void skipComment(string::iterator &text, const string::iterator &until);

            /**
             * check the type of the next block current iterator text points at (not comment or instruction, etc.)
             */
            static block_types nextBlockIs(string::iterator &text, const string::iterator &until);

            /**
             * Skips <? ?>
             */
            static void skipInstruction(string::iterator &text, const string::iterator &until);

            static void getEndBlockName(string &outs, string::iterator text, const string::iterator &until);

            static inline bool isWhitespace(const char &ch);

            /**
             * Searches string iterator for element attributes
             */
            void initAttributes(string::iterator &text, const string::iterator &until);

            friend class ElementTree;
    };

    class ElementTree{
        private:
            shared_ptr<XmlElement> root;

        public:
            static ElementTree* fromString(string &text);

            static ElementTree* fromPath(const filesystem::path &path);

            shared_ptr<XmlElement> getRoot();

            string toString(bool pretty_print=false);
    };
    
    extern string EOL;

    inline bool startsWith(string text, string pattern){
        return (text.rfind(pattern, 0) == 0);
    }

    size_t indexOf(string const& text, string const& value);

    size_t lastIndexOf(string const& text, string const& substring);

    bool contains(string const& text, const string & substring);

    string int_to_hex(int i);

    template<typename V>
    concept is_basic_vector = requires (V v){
        v.at(0);
        to_string(v.at(0));
    };
    template <is_basic_vector T> string vec_to_s(T vec);

    template<typename V>
    concept is_basic_matrix = requires (V v){
        v.at(0);
        v.at(0).at(0);
        to_string(v.at(0).at(0));
    };
    template <is_basic_matrix T> string mat_to_s(T vec);

    bool stob(const string &);

    vector<string> readLines(filesystem::path path);

    vector<vector<double>> parseMat(string text);

    vector<double> parseRow(const string& str);

    string join(vector<string> const& list, string const& delim=" ");

    vector<string> split(string const& text, string const& delim=" ");

    string replaceAll(string const& str, char a, char b);
}

#include "Parser_template_imp.h"

#endif

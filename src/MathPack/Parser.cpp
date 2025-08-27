/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "Parser.h"

using namespace std;
namespace XmlParser{
    /**
     *
     * @author Ivan Ramazanov
     */

    /* XML CLASS */

    XmlElement::XmlElement(const string &name): name(name){}

    XmlElement::~XmlElement(){
        for (auto& elem:children){
            delete elem;
        }
    }

    ElementTree* ElementTree::fromString(string &text){
        auto iterText = text.begin();
        auto until = text.end();

        // cycle through, until proper root block is found
        while (iterText < until){
            if (XmlElement::isWhitespace(*iterText)){
                advance(iterText, 1);
                continue;
            }

            auto type = XmlElement::nextBlockIs(iterText, until);
            if (type == XmlElement::block_types::instruction)
                XmlElement::skipInstruction(iterText, until);
            else if(type == XmlElement::block_types::regular)
                try{
                    ElementTree* ret = new ElementTree();
                    ret->root = shared_ptr<XmlElement>(XmlElement::parseElement(iterText, until));
                    return ret;
                }catch (exception &ex){
                    cerr << "(pos: " << distance(text.begin(), iterText) << ") " << ex.what() << endl;
                    throw runtime_error("failed to parse XML");
                }
            else if (type == XmlElement::block_types::comment)
                XmlElement::skipComment(iterText, until);
            else
                throw runtime_error("XML Parser::failed to parse XML: unknown block type at pos: " +
                                    to_string(distance(text.begin(), iterText)));
        }
        throw runtime_error("failed to parse XML");
    }

    string ElementTree::toString(bool pretty_print){
        return join(root->toStrings(pretty_print), "");
    }

    ElementTree* ElementTree::fromPath(const filesystem::path &path){
        ifstream fs(path);
        if (!fs){
            throw ios_base::failure("Path doesn not exist: " + path.string());
        }

        string content;

        size_t buff_size = 2<<12;
        string buf(buff_size, '\0');
        while(fs.read(&buf[0], buff_size)){
            content.append(buf, 0, fs.gcount());
        }
        content.append(buf, 0, fs.gcount());
        return fromString(content);
    }

    shared_ptr<XmlElement> ElementTree::getRoot(){
        return root;
    }

    XmlElement::block_types XmlElement::nextBlockIs(string::iterator &text, const string::iterator &until){
        if(*text != '<') return block_types::error;
        if (text + 1 < until){
            // currently at '<'
            if (*next(text) == '?') return block_types::instruction;
            // not an instruction block, check comment block
            if (*next(text) == '!'){
                if(text + 3 < until){
                    if (*next(text, 2) == '-' && *next(text, 3) == '-'){
                        return block_types::comment;
                    }
                    // block starts with '<!' which is not allowed (for now)
                    return block_types::error;
                }
                // not enough characters left
                return block_types::error;
            }
            // none of special block matched, so has to be a 'proper' block
            return block_types::regular;
        }
        // not enough characters left
        return block_types::error;
    }

    void XmlElement::append(XmlElement *child){
        children.push_back(move(child));
    }

    vector<XmlElement*> XmlElement::findAll(const string &tag){
        vector<XmlElement*> ret;
        for (auto& child:children){
            if (child->name == tag) ret.push_back(child);
        }
        return ret;
    }

    XmlElement* XmlElement::find(string const& tag){
        for (auto& child:children){
            if (child->name == tag) return child;
        }
        return nullptr;
    }

    XmlElement* XmlElement::parseElement(string::iterator &text, const string::iterator &until){
        // find start of the next block
        while (text < until){
            if (isWhitespace(*text)){
                // skip until we hit '<'
                advance(text, 1);
                continue;
            } else if (*text != '<'){
                // unexpected character - ill formed doc
                throw invalid_argument("XML Parser::document is ill formed: expected start of a block, got: " + *text);
            }

            advance(text, 1);
            while(text < until){
                // have entered block definition

                if (*text == '?'){
                    advance(text, 1);
                    skipInstruction(text, until);
                    continue;
                }else if(*text == '/'){
                    // expected start of the block
                    throw invalid_argument("XML Parser::document is ill formed: unexpected end of a block");
                }else if(*text == '!'){
                    // TODO could be not a comment
                    // skip '--'
                    advance(text, 3);
                    skipComment(text, until);
                    continue;
                }else if(isWhitespace(*text)){
                    throw invalid_argument("XML Parser::document is ill formed: unexpected whitespace at the start of a block name.");
                }

                // special blocks are handled. Only 'real' blcoks are left, so start tracking
                string tempText;
                tempText.reserve(64);

                // get block name
                while (text < until){
                    if (isWhitespace(*text) || *text == '>'){
                        // TODO could end on '/>'
                        // hit end of the name
                        break;
                    }
                    tempText.push_back(*text);
                    advance(text, 1);
                }
                XmlElement* ret = new XmlElement(tempText);
                tempText.clear();

                if (*text != '>'){
                    // get element attributes
                    advance(text, 1);
                    ret->initAttributes(text, until);
                }

                // now we're staying at '>' (end of )
                advance(text, 1);
                bool textTailIsDone = false;
                // capture element's body ('text')
                while (text < until){
                    if (*text != '<'){
                        if (!textTailIsDone){
                            // everything goes into text
                            ret->text.push_back(*text);
                        }
                        advance(text, 1);
                    }else{
                        textTailIsDone = true;
                        // clean up empty tail
                        if (startsWith(ret->text, EOL) || startsWith(ret->text, "\n"))
                            ret->text = "";
                        while (!ret->text.empty() && isWhitespace(ret->text.back()))
                            // remove all whitespaces from the end of text (to prevent extra newline during back conversion)
                            ret->text.pop_back();

                        // hit child block or end of current
                        // check if next is an ending block
                        if (text + 1 < until){
                            // still have string to read
                            if(*next(text) == '/'){
                                // next pattern is '</'
                                // get name of ending
                                advance(text, 2);
                                getEndBlockName(tempText, text, until);
                                if (tempText != ret->name){
                                    throw invalid_argument("XML Parser::invalid ending of block: " + ret->name);
                                }
                                // end is reached. Advance iterator to right after '>'
                                advance(text, tempText.length()+1);
                                return ret;
                            }

                            auto type = nextBlockIs(text, until);
                            if (type == block_types::regular){
                                // next is another block (child)
                                // do recursion
                                ret->append(parseElement(text, until));
                            }else if(type == block_types::comment){
                                skipComment(text, until);
                            }else if(type == block_types::instruction){
                                skipInstruction(text, until);
                            }else throw invalid_argument("XML Parser::unexpected type of child block");
                        }
                    }
                }
            }
        }

        // element should've been finished and returned by now
        throw invalid_argument("XML Parser::document is ill formed: end of a block not found");
    }

    bool XmlElement::isWhitespace(const char &ch){
        return ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r';
    }

    void XmlElement::initAttributes(string::iterator &text, const string::iterator &until){
        while (text < until){
            // check if end of block is reached
            // TODO could end on '/>'
            if (*text == '>') return;

            // skip whitespaces between attributes
            if (isWhitespace(*text)){
                advance(text, 1);
                continue;
            }

            // get next name
            string attrName;
            attrName.reserve(64);
            while (text < until){
                if (*text == '=' || isWhitespace(*text)){
                    break;
                }
                attrName.push_back(*text);
                advance(text, 1);
            }
            if (text >= until) throw runtime_error("XML Parser::failed to parse attribute name: " + attrName);

            // move until "="
            while (text < until && *text != '=')
                advance(text, 1);
            // "=" was found; step over
            advance(text, 1);
            
            // parse attr value
            // skip whitespaces
            while (text < until)
                if (isWhitespace(*text)){
                    advance(text, 1);
                    continue;
                }else{
                    break;
                }
            if (text >= until) throw runtime_error("XML Parser::failed to parse attribute value: " + attrName);

            string attrValue;
            attrValue.reserve(16);
            if (*text == '"'){
                // value is in quotes
                advance(text, 1);
                while (text < until){
                    // until the end quote
                    if (*text == '"'){
                        advance(text, 1);
                        break;
                    }
                    attrValue.push_back(*text);
                    advance(text, 1);
                }
            }else{
                // value in plain text (search for whitespace)
                while(text < until){
                    if (isWhitespace(*text)){
                        advance(text, 1);
                        break;
                    }else if(*text == '>'){
                        // hit the end of block definition
                        break;
                    }
                    attrValue.push_back(*text);
                    advance(text, 1);
                }
            }
            if (text >= until) throw runtime_error("XML Parser::failed to parse attribute value: " + attrName);
            // pair is fully parsed, add attribute
            attributes[attrName] = attrValue;
        }
        if (text >= until) throw runtime_error("XML Parser::failed to parse block attributes.");
    }

    void XmlElement::getEndBlockName(string &outs, string::iterator text, const string::iterator &until){
        while (text < until){
            if(*text == '>'){
                return;
            }
            outs.push_back(*text);
            advance(text, 1);
        }
        throw runtime_error("XML Parser::failed to find end of a block.");
    }

    /**
     * Skips <!-- -->
     */
    void XmlElement::skipComment(string::iterator &text, const string::iterator &until){
        while (text + 2 < until){
            if (*text == '-' && *next(text, 1) == '-' && *next(text, 2) == '>'){
                // found the end pattern
                advance(text, 3);
                return;
            }
            advance(text, 1);
        }
        throw runtime_error("XML Parser:: can't find the end of comment block.");
    }

    /**
     * Skips <? ?>
     */
    void XmlElement::skipInstruction(string::iterator &text, const string::iterator &until){
        while (text + 1 < until){
            if (*text == '?' && *next(text, 1) == '>'){
                // found the end pattern
                advance(text, 2);
                return;
            }
            advance(text, 1);
        }
        throw runtime_error("XML Parser:: can't find the end of instruction block.");
    }

    vector<string> XmlElement::toStrings(bool pretty_print){
        static size_t offset = 0;
        vector<string> ret;
        
        // self
        string info;
        for (auto i = offset; i-->0;)
            info.append("    ");
        info.append("<" + name);
        
        for (const auto& [n, v]:attributes){
            info.append(" " + n + "=\"" + v + "\"");
        }

        info.append(">" + text);
        if (pretty_print && !children.empty()) info.append(EOL);
        ret.push_back(info);

        // recursion into children
        for (auto& child:children){
            if (pretty_print) offset++;
            vector<string> child_info = child->toStrings(pretty_print);
            ret.insert(ret.end(), child_info.begin(), child_info.end());
            if (pretty_print) offset--;
        }

        // ending
        info.clear();
        if (pretty_print && !children.empty()){
            // newline was inserted, therefore repeat offset
            for (auto i = offset; i-->0;)
                info.append("    ");
        }
        info.append("</" + name + ">");
        if (pretty_print) info.append(EOL);
        ret.push_back(info);

        return ret;
    }
    /* END OF XML CLASS*/

    string EOL = "\r\n";

    size_t indexOf(string const& text, string const& value){
        // auto pos = find(text.begin(), text.end(), value);
        // if (pos == text.end()){
        //     return string::npos;
        // }
        // return distance(text.begin(), pos);
        return text.find(value);
    }

    size_t lastIndexOf(string const& text, const string &substring){
        // auto pos = find(text.rbegin(), text.rend(), substring);
        // if (pos == text.rend()){
        //     return string::npos;
        // }
        // return distance(pos, text.rend())-1;
        return text.rfind(substring);
    }

    bool contains(string const& text, const string & substring){
        return indexOf(text, substring) != string::npos;
    }

    string int_to_hex(int i){
        stringstream stream;
        stream << setfill('0') << setw(sizeof(int)*2) << hex << i;
        return stream.str();
    }

    bool stob(const string& value){
        if (value == "true" || value == "1"){
            return true;
        }else if (value == "false" || value == "0"){
            return false;
        }else{
            throw invalid_argument("Invalid boolean value: " + value);
        }
    }

    vector<string> readLines(filesystem::path path){
        vector<string> out;
        ifstream fs(path);
        string temp;
        while(getline(fs, temp)){
            if(temp.back() == '\r'){
                // remove
                temp.pop_back();
            }
            out.push_back(temp);
        }
        return out;
    }

    /**
     * parse matrix from string input
     * @param text either in matrix form: [x,y, ...; a b...] or scalar: x
     */
    vector<vector<double>> parseMat(string text){
        vector<vector<double>> out;

        if(text.at(0) == '['){
            vector<string> lines;
            if(text[text.length()-1] == ']'){
                // remove brackets
                // TODO: TEST THIS!!!
                text = text.substr(1, text.length()-2);

                lines = split(text, ";");
                // TODO filter empty elements?
            }else{
                throw invalid_argument("XmlParse::parse::Corrupted parameter! " + text);
            }
            size_t numberOfRows = lines.size();

            // now we have row of lines
            size_t i=0;
            size_t numberOfCols = size_t(-1);
            for(auto& row:lines){
                string temp;
                vector<string> cols;
                for(size_t j=0; j<row.length(); j++){
                    char c = row[j];
                    if(c!=',' && c!=' '){
                        temp+=c;
                    }else{
                        cols.push_back(temp);
                        temp="";
                    }
                }
                if(!temp.empty()){
                    cols.push_back(temp);
                }

                if(numberOfCols != size_t(-1)){
                    if(numberOfCols != cols.size()){
                        throw invalid_argument("Dimensions mismatch!\n Parameter:  line: " + row + "\n in " + text);
                    }
                }else{
                    // first loop cycle. init output
                    numberOfCols = cols.size();
                    out = vector<vector<double>>(numberOfRows, vector<double>(numberOfCols));
                }

                // cast to array
                int j=0;
                for(auto& col:cols){
                    out[i][j] = stod(col);
                    j++;
                }
                i++;
            }

        }else{
            // scalar case ([1,1] dimensional out)
            out = vector<vector<double>> (1, vector<double>(1, stod(text)));
        }
        return out;
    }

    vector<double> parseRow(const string& str){
        vector<vector<double>> tmp = parseMat(str);

        if(tmp.size()>1 && tmp[0].size()>1) {
            throw invalid_argument("Dimensions mismatch in . Must be a vector (but matrix).");
        }else {
            if(tmp.size() > 1){
                vector<double> out(tmp.size());
                // has to be [m x 1] vector, convert to row (singular 'vector')
                for (size_t i=0; i<tmp.size(); i++){
                    out[i] = tmp[i][0];
                }
                return out;
            }
        }

        // tmp is a row, so just return it as is
        return tmp[0];
    }

    string rgbToHash(string const& rgb){
        string R,G,B;
        int r,g,b;
        string divider = ".";
        size_t i = indexOf(rgb, divider);
        if(i == string::npos) {
            divider = ",";
            if ((i = indexOf(rgb, divider)) == string::npos){
                throw invalid_argument("Wrong RGB formatting in: " + rgb);
            }
        }

        R = rgb.substr(0, i);
        G = rgb.substr(i+1);
        i = indexOf(G, divider);
        if(i==string::npos){
            throw invalid_argument("Wrong RGB formatting in:" +rgb);
        }
        B = G.substr(i+1);
        G = G.substr(0, i);

        //try{

        r = stoi(R);
        g = stoi(G);
        b = stoi(B);
        string out("#");
        // TODO: use local function
        //R
        if(r<256){
            out += int_to_hex(r);
        }else{
            throw invalid_argument("R in rgb greater than 255!: " + rgb);
        }

        //G
        if(g<256){
            out += int_to_hex(g);
        }else{
            throw invalid_argument("G in rgb greater than 255!: " + rgb);
        }

        //B
        if(b<256){
            out += int_to_hex(b);
        }else{
            throw invalid_argument("B in rgb greater than 255!: " + rgb);
        }

        //output
        return out;

        //}catch(invalid_argument & ex){
        //    cerr << ex.what();
        //}
        //return null;
    }

    void removeTab(string & text){
        if(text.length() > 0 && text[0] == '\t'){
            text = text.substr(1);
        }else if(text.length() > 3 && text[0] == ' ' && text[1] == ' ' && text[2] == ' ' && text[3] == ' '){
            text = text.substr(4);
        }
    }

    string join(vector<string> const& list, string const& delim){
        if (list.empty()){
            return string("");
        }
        
        string ret(list[0]);
        
        for (size_t i=1; i < list.size(); i++){
            ret += delim + list[i];
        }

        return ret;
    }

    vector<string> split(string const& text, string const& delim){
        vector<string> ret;
        if (text.empty()){
            return ret;
        }

        size_t  delim_pos,
                current_pos=0;
        while((delim_pos = text.find(delim, current_pos)) != string::npos){
            ret.push_back(text.substr(current_pos, delim_pos-current_pos));
            current_pos = delim_pos + delim.length();
        }

        // rest of the string (or empty 'text')
        ret.push_back(text.substr(current_pos, text.length()-current_pos));

        return ret;
    }

    string replaceAll(string str, char a, char b){
        for (char& c:str){
            if (c == a)
                c = b;
        }
        return str;
    }
}

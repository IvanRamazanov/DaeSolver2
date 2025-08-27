#include "Parser.h"

namespace XmlParser{
    template <is_basic_vector T> string vec_to_s(T vec){
        string ret="[";
        //size_t last = vec.size() - 1;
        for (auto& val:vec){
            ret += to_string(val);
            ret.push_back(',');
        }
        if (ret.back() == ',')
            ret.back() = ']';
        else
            // empty vector, just add closing bracket
            ret.push_back(']');
        return ret;
    }

    template <is_basic_matrix T> string mat_to_s(T vec){
        string ret="[";
        for (auto& row:vec){
            for (auto& val:row){
                ret += to_string(val);
                ret.push_back(',');
            }
            if (ret.back() == ',')
                ret.back() = ';';
            else
                // empty vector, just add closing bracket
                ret.push_back(';');
        }
        if (ret.back() == ';')
            ret.back() = ']';
        else
            // empty matrix, just add closing bracket
            ret.push_back(']');
        return ret;
    }
}

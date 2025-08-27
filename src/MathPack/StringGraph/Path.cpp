#include "../StringGraph.h"

namespace MathPack{
    /**
     * Path class definition
     */
    Path::Path(){}

    void Path::addPoint(node_container<FuncNode> func, size_t numOfOper){
        uzelPath.push_back(func);
        numberOfOperand.push_back(numOfOper);
    }

    const vector<node_container<FuncNode>>& Path::getFuncs(){
        return uzelPath;
    }

    size_t Path::length(){
        return uzelPath.size();
    }

    /**
     * Get i-th node in the path
     */
    node_container<FuncNode> Path::getNode(size_t const idx){
        return uzelPath.at(idx);
    }

    /**
     * Note: used for debug purposes
     */
    string Path::toString(){
        string ret;
        for(size_t i=0; i<uzelPath.size(); i++){
            ret += " " + to_string((size_t)to_address(uzelPath[i])) + " oper:" + to_string(numberOfOperand[i]);
        }
        return ret;
    }
}

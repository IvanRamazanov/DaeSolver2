#ifndef MATHPACK_SG_TMPLT_H
#define MATHPACK_SG_TMPLT_H

#include "StringGraph.h"

namespace MathPack{
    inline Node* pack_to_node(const std::shared_ptr<Node>& ptr) {
        return ptr.get();  // assuming Node::copy() returns Node*
    }

    inline Node* pack_to_node(Node* ptr) {
        return ptr;
    }

    /**
     * FuncNode class definition
     */
    // template<node_vector... T>
    // FuncNode::FuncNode(string const& function,  const T&... input){
    //     func = MathFunction(function);
    //     ((
    //         inputs.push_back(pack_to_node(input)->copy()),
    //         gain.push_back(1)
    //     ), ...);

    //     // ???
    //     if(function == "-" || function == "/"){
    //         throw invalid_argument("Invalid function identifier!");
    //     }

    //     this->input.resize(this->inputs.size());
    //     //simplify();   DOESN'T WORK!
    // }

    template<typename... T>
    FuncNode::FuncNode(string const& function,  const node_container<T>&... input){
        func = MathFunction(function);
        ((
            inputs.push_back(input),
            gain.push_back(1)
        ), ...);

        // ???
        if(function == "-" || function == "/"){
            throw invalid_argument("Invalid function identifier!");
        }

        this->input.resize(this->inputs.size());
        //simplify();   DOESN'T WORK!
    }
}

#endif

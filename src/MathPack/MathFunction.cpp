#include <cmath>
#include <stdexcept>
#include "MathFunction.h"

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

using namespace std;

namespace MathPack{

    MathFunction::MathFunction(){
        // Note, only for FuncNode constructor
    }

    MathFunction::MathFunction(string const& functionName){
        name = functionName;

        if(name == "+"){
            rank=-1;
            function = make_shared<Summa>();
        }else if(name == "sin"){
            rank=1;
            function = make_shared<Sin>();
        }else if(name == "*"){
            rank=-1;
            function = make_shared<Multiply>();
        }else if(name == "arcSin"){
            rank=1;
            function = make_shared<ArcSin>();
        }else if(name == "exp"){
            rank=1;
            function = make_shared<Exp>();
        }else if(name == "logn"){
            rank=1;
            function = make_shared<Logn>();
        }else if(name == "gr"){
            rank=2;
            function = make_shared<GreatThan>();
        }else if(name == "if"){
            rank=3;
            function = make_shared<If>();
        }else if(name == "inv"){
            rank=1;
            function = make_shared<Inverse>();
        }else if(name == "cos"){
            rank=1;
            function = make_shared<Cos>();
        }else if(name == "pow"){ // || name == "^"
            rank=2;
            function = make_shared<Pow>();
        }else{
            throw invalid_argument("Unsupported math function: "+functionName);
        }
    }

    // MathFunction::MathFunction(MathFunction const& input){
    //     this->rank = input.rank;
    //     this->function = input.function->_copy();
    //     this->name = input.name;
    // }

    // MathFunction& MathFunction::operator=(MathFunction const& input){
    //     this->name = input.name;
    //     this->rank = input.rank;
    //     this->function = input.function->_copy();

    //     return *this;
    // }

    // MathFunction::MathFunction(MathFunction && input){
    //     this->name = input.name;
    //     this->rank = input.rank;

    //     this->function = input.function;
    //     input.function = nullptr;
    // }

    // MathFunction& MathFunction::operator=(MathFunction && input){
    //     this->name = input.name;
    //     this->rank = input.rank;

    //     this->function = input.function;
    //     input.function = nullptr;

    //     return *this;
    // }

    // MathFunction::~MathFunction(){
    //     delete function;
    // }

    bool MathFunction::isSimple(MathFunction *inp){
        auto type = inp->function->type();
        return type == SimpleFunc::FuncTypes::tSum || type == SimpleFunc::FuncTypes::tMult;
    }

    void MathFunction::inverse(int index){
        //delete function;
        function = function->_inverse(index);
    }

    const string& MathFunction::getFuncName(){
        return name;
    }

    string MathFunction::getFuncName(int i){
        return function->getName(i);
    }

    SimpleFunc::FuncTypes MathFunction::getFuncType(){
        return function->type();
    }

    node_container<Node> MathFunction::simplify(node_container<FuncNode> inp){
        return function->simplify(inp);
    }

    node_container<Node> MathFunction::expand(node_container<FuncNode> inp){
        return function->expand(inp);
    }

    double MathFunction::Evaluate(vector<int> gains, vector<double> input){
        return function->Evaluate(gains, input);
    }

    int MathFunction::getRank(){
        return rank;
    }

    node_container<Node> MathFunction::differ(node_container<FuncNode> root, string const& varName){
        return function->differ(root, varName);
    }

    bool MathFunction::contain(vector<node_container<Node>> inp, string const& varName){
        vector<int> indexes = function->getRequiredIndexes();
        bool out = false;
        if(indexes[0] != -1){
            for(int i:indexes){
                if(inp.at(i)->contains(varName)){
                    out=true;
                    break;
                }
            }
        }else{
            for(auto& uz:inp){
                if(uz->contains(varName)){
                    out=true;
                    break;
                }
            }
        }
        return out;
    }
}

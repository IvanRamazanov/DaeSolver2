#include "../MathFunction.h"

namespace MathPack{
    double Exp::Evaluate(vector<int> const& gains, vector<double> const& input) {
        if(gains.at(0)==1){
            return exp(input[0]);
        }else{
            return log(input[0]);
        }
    }

    shared_ptr<SimpleFunc> Exp::_inverse(int index) {
        return make_shared<MathPack::Logn>();
    }

    SimpleFunc::FuncTypes Exp::type(){
        return FuncTypes::tExp;
    }

    string Exp::getName(int i) {
        if(i==1){
            return "exp";
        }else{
            return "logn";
        }
    }

    node_container<Node> Exp::simplify(node_container<FuncNode> input) {
        node_container<Node> output = nullptr;
        auto &inps = input->getInputs();
        auto &gains = input->getGain();

        //simplify inputs
        for(int i=0; i<inps.size(); i++){
            auto &uz = inps[i];
            if(uz->type() == FunctionNode){
                auto rp = cast_node<FuncNode>(uz);
                input->setInput(rp->simplify(), i);
            }
        }

        // decrease consts
        size_t numOfConsts=0;
        for(int i=0; i<inps.size(); i++){
            auto &uz = inps[i];
            if(uz->type() == ConstNode){
                numOfConsts++;
            }
        }
        if(numOfConsts==inps.size()){
            output = make_node<Const>(Evaluate(gains, {cast_node<Const>(inps.at(0))->getValue()}));
            clear_node(input);
        }else{
            output = input;
        }

        return output;
    }

    node_container<Node> Exp::expand(node_container<FuncNode> uz){
        return uz;
    }

    vector<int> Exp::getRequiredIndexes() {
        return {-1};
    }

    node_container<Node> Exp::differ(node_container<FuncNode> root, string const& varName) {
        auto b = root->getInputs().at(0)->differ(varName);
        return make_node<FuncNode>("*", root, b);
    }
}

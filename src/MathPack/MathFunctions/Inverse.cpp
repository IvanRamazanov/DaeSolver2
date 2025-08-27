#include "../MathFunction.h"

namespace MathPack{
    double Inverse::Evaluate(vector<int> const& gains, vector<double> const& input) {
        if(gains.at(0) == 1)
            return 1/input[0];
        else
            return input[0];
    }

    shared_ptr<SimpleFunc> Inverse::_inverse(int index) {
        throw runtime_error("Not supported yet.");
    }

    SimpleFunc::FuncTypes Inverse::type(){
        return FuncTypes::tInverse;
    }

    string Inverse::getName(int i) {
        return "inv";
    }

    node_container<Node> Inverse::simplify(node_container<FuncNode> input) {
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
        for(int i=0;i<inps.size();i++){
            auto &uz = inps[i];
            if(uz->type() == ConstNode){
                numOfConsts++;
            }
        }
        if(numOfConsts==inps.size()){
            output = make_node<Const>(Evaluate(gains, {cast_node<Const>(inps.at(0))->getValue()}));
            clear_node(input);
        }else{
            output=input;
        }

        return output;
    }

    node_container<Node> Inverse::expand(node_container<FuncNode> uz){
        return uz;
    }

    vector<int> Inverse::getRequiredIndexes() {
        return {-1};
    }

    node_container<Node> Inverse::differ(node_container<FuncNode> root, string const& varName) {
        if(root->getGain().at(0)==1){
            auto out=make_node<FuncNode>("pow", root->getInputs().at(0)->copy(), make_node<Const>(-2.0));
            out=make_node<FuncNode>("*", out, make_node<Const>(-1.0), root->getInputs().at(0)->differ(varName));
            return out;
        }else{
            return root->getInputs().at(0)->differ(varName);
        }
    }
}

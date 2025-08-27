#include "../MathFunction.h"

namespace MathPack{
    double If::Evaluate(vector<int> const& gains, vector<double> const& input) {
        if(input[0]==0){
            return input[2];
        }else{
            return input[1];
        }
    }

    shared_ptr<SimpleFunc> If::_inverse(int index) {
        throw runtime_error("Not supported yet.");
    }

    SimpleFunc::FuncTypes If::type(){
        return FuncTypes::tIF;
    }

    string If::getName(int i) {
        return "if";
    }

    node_container<Node> If::simplify(node_container<FuncNode> input) {
        node_container<Node> output = nullptr;
        auto &inps = input->getInputs();
        auto &gains = input->getGain();

        //simplify inputs
        for(int i=0;i<inps.size();i++){
            auto &uz = inps[i];
            if(uz->type() == FunctionNode){
                auto rp = cast_node<FuncNode>(uz);
                input->setInput(rp->simplify(), i);
            }
        }

        // decrease consts
        if(inps.at(0)->type() == ConstNode){
            if(cast_node<Const>(inps.at(0))->getValue() == 0){
                output = inps.at(2)->copy();
            }else{
                output = inps.at(1)->copy();
            }
            clear_node(input);
        }else if(inps.at(1)->toString() == inps.at(2)->toString()){
            output = inps[1]->copy();
            clear_node(input);
        }else{
            output = input;
        }   

        return output;
    }

    node_container<Node> If::expand(node_container<FuncNode> uz){
        return uz;
    }

    vector<int> If::getRequiredIndexes() {
        return {1, 2};
    }

    node_container<Node> If::differ(node_container<FuncNode> root, string const& varName) {
        node_container<FuncNode> out = cast_node<FuncNode>(root->copy());
        auto &inps = out->getInputs();
        out->setInput(inps.at(1)->differ(varName), 1);
        out->setInput(inps.at(2)->differ(varName), 2);
        
        return out;
    }
}

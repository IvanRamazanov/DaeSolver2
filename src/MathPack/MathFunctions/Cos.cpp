#include "../MathFunction.h"

namespace MathPack{
    double Cos::Evaluate(vector<int> const& gains, vector<double> const& input) {
        return cos(input[0]);
    }

    shared_ptr<SimpleFunc> Cos::_inverse(int index) {
        throw runtime_error("Not supported yet.");
    }

    SimpleFunc::FuncTypes Cos::type(){
        return FuncTypes::tCos;
    }

    string Cos::getName(int i) {
        return "cos";
    }

    node_container<Node> Cos::simplify(node_container<FuncNode> input) {
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
            clear_node(output);
        }else{
            output = input;
        }

        return output;
    }

    node_container<Node> Cos::expand(node_container<FuncNode> uz){
        return uz;
    }

    vector<int> Cos::getRequiredIndexes() {
        return {0};
    }

    node_container<Node> Cos::differ(node_container<FuncNode> root, string const& varName) {
        auto out = make_node<FuncNode>("sin", root->getInputs(), root->getGain());
        auto b = root->getInputs().at(0)->differ(varName);
        out = make_node<FuncNode>("*", out, make_node<Const>(-1.0), b);
        return out;
    }
}

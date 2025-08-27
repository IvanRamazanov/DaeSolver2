#include "../MathFunction.h"

namespace MathPack{
    double Pow::Evaluate(vector<int> const& gains, vector<double> const& input) {
        // pow() requres 2 arguments, but operator ^ can take any number.
        // Note: uses direct order of calculation: -->
        double ret = pow(input[0], input[1]);
        // for (size_t i=2; i<input.size(); i++)
        //     ret = pow(ret, input[i]);
        return ret;
    }

    shared_ptr<SimpleFunc> Pow::_inverse(int index) {
        throw runtime_error("Not supported yet."); //To change body of generated methods, choose Tools | Templates.
    }

    SimpleFunc::FuncTypes Pow::type(){
        return FuncTypes::tPow;
    }

    string Pow::getName(int i) {
        return "pow";
    }

    node_container<Node> Pow::simplify(node_container<FuncNode> input) {
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
        size_t numOfConsts=0;
        for(int i=0; i<inps.size(); i++){
            auto &uz = inps[i];
            if(uz->type() == ConstNode){
                numOfConsts++;
            }
        }
        if(numOfConsts==inps.size()){
            vector<double> vals(inps.size());
            for(int i=0; i<inps.size(); i++){
                vals[i] = cast_node<Const>(inps[i])->getValue();
            }
            output = make_node<Const>(Evaluate(gains, vals));
            clear_node(input);
        }else{
            output = input;
        }

        return output;
    }

    node_container<Node> Pow::expand(node_container<FuncNode> uz){
        return uz;
    }

    vector<int> Pow::getRequiredIndexes() {
        return {0};
    }

    node_container<Node> Pow::differ(node_container<FuncNode> root, string const& varName) {
        double oldval = cast_node<Const>(root->getInputs().at(1))->getValue();
        auto dInp0 = root->getInputs().at(0)->differ(varName);

        vector<node_container<Node>> inps = {root->getInputs().at(0)->copy(),
                                            make_node<Const>(oldval-1.0)};
        auto out = make_node<FuncNode>("pow", inps);

        out = make_node<FuncNode>("*", out, make_node<Const>(oldval), dInp0);
        return out;
    }
}

#include "../MathFunction.h"

namespace MathPack{
    double ArcSin::Evaluate(vector<int> const& gains, vector<double> const& input) {
        if(gains.at(0) == 1){
            return asin(input[0]);
        }else{
            return sin(input[0]);
        }
    }

    shared_ptr<SimpleFunc> ArcSin::_inverse(int index) {
        return make_shared<Sin>();
    }

    SimpleFunc::FuncTypes ArcSin::type(){
        return FuncTypes::tASin;
    }

    string ArcSin::getName(int i) {
        if(i==1){
            return "asin";
        }else{
            return "sin";
        }
    }

    node_container<Node> ArcSin::simplify(node_container<FuncNode> input) {
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
        for(int i=0;i<inps.size();i++){
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

    node_container<Node> ArcSin::expand(node_container<FuncNode> uz){
        return uz;
    }

    vector<int> ArcSin::getRequiredIndexes() {
        return {-1};
    }

    node_container<Node> ArcSin::differ(node_container<FuncNode> root, string const& varName) {
        throw runtime_error("Not supported yet.");
    }
}

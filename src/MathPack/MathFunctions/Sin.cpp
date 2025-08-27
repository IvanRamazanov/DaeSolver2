#include "../MathFunction.h"

namespace MathPack{
    double Sin::Evaluate(vector<int> const& gains, vector<double> const& input) {
        if(gains.at(0)==1){
            return sin(input.at(0));
        }else{
            return asin(input.at(0));
        }
    }

    shared_ptr<SimpleFunc> Sin::_inverse(int index) {
        return make_shared<ArcSin>();
    }

    SimpleFunc::FuncTypes Sin::type(){
        return FuncTypes::tSine;
    }

    string Sin::getName(int i) {
        if(i==1){
            return "sin";
        }else{
            return "asin";
        }
    }

    node_container<Node> Sin::simplify(node_container<FuncNode> input) {
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
        size_t numOfConsts = 0;
        for(int i=0; i<inps.size(); i++){
            auto &uz = inps[i];
            if(uz->type() == ConstNode){
                numOfConsts++;
            }
        }
        if(numOfConsts == inps.size()){
            output = make_node<Const>(Evaluate(gains, {cast_node<Const>(inps.at(0))->getValue()}));
            clear_node(input);
        }else{
            output=input;
        }

        return output;
    }

    node_container<Node> Sin::expand(node_container<FuncNode> uz){
        return uz;
    }

    vector<int> Sin::getRequiredIndexes() {
        return {-1};
    }

    node_container<Node> Sin::differ(node_container<FuncNode> root, string const& varName) {
        vector<node_container<Node>> inps(root->getRank());
        auto gains = root->getGain();
        for (auto i=root->getInputs().size(); i-->0;)
            inps[i] = root->getInputs()[i]->copy();

        auto a = make_node<FuncNode>("cos", inps, gains);
        auto b = root->getInputs().at(0)->differ(varName);
        return make_node<FuncNode>("*", a, b);
    }
}

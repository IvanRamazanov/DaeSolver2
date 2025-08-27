#include "../MathFunction.h"

namespace MathPack{
    double GreatThan::Evaluate(vector<int> const& gains, vector<double> const& input) {
        if(input[0]>input[1]) return 1;
        else return 0;
    }

    shared_ptr<SimpleFunc> GreatThan::_inverse(int index) {
        throw runtime_error("Not supported yet.");
    }

    SimpleFunc::FuncTypes GreatThan::type(){
        return FuncTypes::tGZ;
    }

    string GreatThan::getName(int i) {
        return "gr";
    }

    node_container<Node> GreatThan::simplify(node_container<FuncNode> input) {
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
            auto v1 = cast_node<Const>(inps.at(0))->getValue(),
                v2 = cast_node<Const>(inps.at(1))->getValue();
            output = make_node<Const>(Evaluate(gains, {v1, v2}));
            clear_node(input);
        }else{
            output = input;
        }

        return output;
    }

    node_container<Node> GreatThan::expand(node_container<FuncNode> uz){
        return uz;
    }

    vector<int> GreatThan::getRequiredIndexes() {
        return {-1};
    }

    node_container<Node> GreatThan::differ(node_container<FuncNode> root, string const& varName) {
        return root->copy();
    }
}

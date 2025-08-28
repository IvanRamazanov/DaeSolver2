#include "../MathFunction.h"

namespace MathPack{
    double Subtraction::Evaluate(vector<int> const& gains, vector<double> const& input) {
        double output=0;
        for(int i=0; i<input.size(); i++){
            if(gains.at(i) == 1){
                output-=input[i];
            }else{
                output+=input[i];
            }
        }
        return output;
    }

    shared_ptr<SimpleFunc> Subtraction::_inverse(int index) {
        if(index==0){
            return make_shared<Summa>();
        }else{
            return make_shared<Subtraction>();
        }
    }

    SimpleFunc::FuncTypes Subtraction::type(){
        return FuncTypes::tSubtract;
    }

    node_container<Node> Subtraction::simplify(node_container<FuncNode> input) {
        node_container<Node> output = nullptr;
        auto &inps = input->getInputs();
        auto &gains = input->getGain();
        size_t len = inps.size();

        //simplify inputs
        for(int i=0; i<inps.size(); i++){
            if(inps[i]->type() == FunctionNode){
                auto rp = cast_node<FuncNode>(inps[i]);
                input->setInput(rp->simplify(), i);
            }
        }

        //gathering
        for(size_t i=0; i<len; i++){
            if(inps[i]->type() == FunctionNode){
                auto fuz = cast_node<FuncNode>(inps[i]);
                if(fuz->getFuncName() == "-"){
                    len += fuz->getInputs().size()-1;

                    for(size_t j=0; j<fuz->getRank(); j++){
                        input->addOperand(fuz->getInputs()[j], fuz->getGain()[j]);
                        // unput was moved; prevent deallocation
                        fuz->setInput(nullptr, j);
                    }

                    input->removeInp(i);
                    i--;
                }
            }
        }

        // decrease consts
        size_t numOfConsts=0;
        vector<double> inputs(input->getRank());
        vector<int> indxs(input->getRank());
        for(size_t i=0; i<input->getRank(); i++){
            if(inps[i]->type() == ConstNode){
                inputs[numOfConsts] = cast_node<Const>(inps[i])->getValue();
                indxs[numOfConsts] = i;
                numOfConsts++;
            }
        }
        if(numOfConsts == inps.size()){
            output=make_node<Const>(Evaluate(gains, inputs));
            clear_node(input);
        }else if(inps.size() == 1){
            if(gains.at(0) == 1){
                output = inps[0]->copy();
                clear_node(input);
            }else{
                output = input;
            }
        }else if(numOfConsts==1){
            if(gains.at(indxs[0])*inputs[0] == 0.0){
                input->removeInp(indxs[0]);
            }
            output = input;
        }else{
            double val=0;
            for(size_t i=0; i<numOfConsts; i++){
                val -= gains.at(indxs[i])*inputs[i];
            }
            input->addOperand(make_node<Const>(val), 1);
            for(size_t i=numOfConsts;i-->0;){
                input->removeInp(indxs[i]);
            }
            output = input;
        }

        return output;
    }

    node_container<Node> Subtraction::expand(node_container<FuncNode> uz){
        return uz;
    }

    string Subtraction::getName(int i) {
        if(i==1){
            return "-";
        }else{
            return "+";
        }
    }

    vector<int> Subtraction::getRequiredIndexes() {
        return {-1};
    }

    node_container<Node> Subtraction::differ(node_container<FuncNode> root, string const& varName) {
        vector<node_container<Node>> inps(root->getRank());
        vector<int> gains(root->getRank());

        for(auto i=root->getInputs().size(); i-->0;){
            inps[i] = root->getInputs()[i]->differ(varName);
            gains[i] = root->getGain()[i];
        }
        auto ret = make_node<FuncNode>("-", inps, gains);
        return ret->simplify();
    }
}

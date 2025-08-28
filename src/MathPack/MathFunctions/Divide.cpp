#include "../MathFunction.h"

namespace MathPack{
    double Divide::Evaluate(vector<int> const& gains, vector<double> const& input) {
        double output=input[0];
        if(gains.at(0) == 1)
            output=1/output;
        for(int i=1; i<input.size(); i++){
            if(gains.at(i) == 1){
                output/=input[i];
            }else{
                output*=input[i];
            }
        }
        return output;
    }

    shared_ptr<SimpleFunc> Divide::_inverse(int index) {
        if(index == 0)
            return make_shared<Multiply>();
        else
            return make_shared<Divide>();
    }

    SimpleFunc::FuncTypes Divide::type(){
        return FuncTypes::tDivide;
    }

    node_container<Node> Divide::simplify(node_container<FuncNode> input) {
        node_container<Node> output = nullptr;
        auto &inps = input->getInputs();
        auto &gains = input->getGain();
        int len = inps.size();

        //simplify inputs
        for(int i=0; i<inps.size(); i++){
            auto &uz = inps[i];
            if(uz->type() == FunctionNode){
                auto rp = cast_node<FuncNode>(uz);
                input->setInput(rp->simplify(), i);
            }
        }

        //gathering
        for(int i=0; i<len; i++){
            if(inps[i]->type() == FunctionNode){
                auto fuz = cast_node<FuncNode>(inps[i]);
                if(fuz->getFuncName() == "/"){
                    len += fuz->getRank()-1;

                    // TODO: are you sure gain[i] shouldn't be taken into account??
                    for(size_t j=0; j<fuz->getRank(); j++){
                        input->addOperand(fuz->getInputs()[j], fuz->getGain()[j]);
                        // input was moved; prevent deallocation
                        fuz->setInput(nullptr, j);
                    }

                    // i-th inp was moved up 1 level; erase
                    input->removeInp(i);
                    i--;
                }
            }
        }

        // reduce consts
        size_t numOfConsts=0;
        vector<double> inputs(input->getRank());
        vector<int>    indxs(input->getRank());
        for(size_t i=0; i<input->getRank(); i++){
            if(inps[i]->type() == ConstNode){
                inputs[numOfConsts] = cast_node<Const>(inps[i])->getValue();
                indxs[numOfConsts] = i;
                numOfConsts++;
            }
        }
        if(numOfConsts==inps.size()){
            output = make_node<Const>(Evaluate(gains, inputs));
            clear_node(input);
        }else if(inps.size()==1){
            if(gains.at(0)==1){
                output = inps[0]->copy();
                clear_node(input);
            }else{
                output = input;
            }
        }else{
            double val=1;
            for(size_t i=0; i<numOfConsts; i++){
                val *= pow(inputs[i], gains.at(indxs[i])*-1);
            }
            input->addOperand(make_node<Const>(val), 1);

            for(size_t i=numOfConsts; i-->0;){
                input->removeInp(indxs[i]);
            }

            output = input;
        }

        return output;
    }

    node_container<Node> Divide::expand(node_container<FuncNode> uz){
        return uz;
    }

    string Divide::getName(int i) {
        if(i==1){
            return "/";
        }else{
            return "*";
        }
    }

    vector<int> Divide::getRequiredIndexes() {
        return {-1};
    }

    node_container<Node> Divide::differ(node_container<FuncNode> root, string const& varName) {
        auto inps = root->getInputs();
        auto gains = root->getGain();
        node_container<Node> a,b;

        if(gains.at(0)==1){
            a = make_node<FuncNode>("inv", inps.at(0)->copy());
        }else{
            a = inps.at(0)->copy();
        }

        if (root->getRank() == 1){
            // TODO ???
            throw runtime_error("Divide::Differ - not yet implemented!");
        }else if(root->getRank()==2){
            if(gains.at(1) == 1){
                b = make_node<FuncNode>("inv", inps.at(1)->copy());
            }else{
                b = inps.at(1)->copy();
            }
        }else{
            vector<node_container<Node>> sub_i;
            vector<int> sub_g;
            for (size_t i=1; i<root->getRank(); i++){
                sub_i.push_back(inps[i]->copy());
                sub_g.push_back(gains[i]);
            }

            b = make_node<FuncNode>("*", sub_i, sub_g);
        }

        auto left = make_node<FuncNode>("*", a->differ(varName), b);
        auto right = make_node<FuncNode>("*", a, b->differ(varName));

        return make_node<FuncNode>("+", left, right);
    }
}

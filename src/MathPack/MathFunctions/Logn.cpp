#include "../MathFunction.h"

namespace MathPack{
    double Logn::Evaluate(vector<int> const& gains, vector<double> const& input) {
        if(gains.at(0)==1){
            return log(input[0]);
        }else{
            return exp(input[0]);
        }
    }

    shared_ptr<SimpleFunc> Logn::_inverse(int index) {
        return make_shared<Exp>();
    }

    SimpleFunc::FuncTypes Logn::type(){
        return FuncTypes::tLogn;
    }

    string Logn::getName(int i) {
        if(i==1){
            return "logn";
        }else{
            return "exp";
        }
    }

    node_container<Node> Logn::simplify(node_container<FuncNode> input) {
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
            output=make_node<Const>(Evaluate(gains, {cast_node<Const>(inps.at(0))->getValue()}));
            clear_node(input);
        }else{
            output=input;
        }

        return output;
    }

    node_container<Node> Logn::expand(node_container<FuncNode> uz){
        return uz;
    }

    vector<int> Logn::getRequiredIndexes() {
        return {-1};
    }

    /**
     * (ln(x))'=(1/x)*(x')
     * a=1/x, b=x'.
     * @param root
     * @param varName
     * @return
     */
    node_container<Node> Logn::differ(node_container<FuncNode> root, string const& varName) {
        vector<node_container<Node>> inps(root->getRank());
        vector<int> gains(root->getRank());
        for (size_t i=0; i<root->getRank(); i++){
            inps[i] = root->getInputs()[i]->copy();
            gains[i] = root->getGain()[i];
        }
        auto a = make_node<FuncNode>("inv", inps, gains);
        auto b = root->getInputs().at(0)->differ(varName);
        return make_node<FuncNode>("*", a, b);
    }
}

#include "../MathFunction.h"

namespace MathPack{
    double Multiply::Evaluate(vector<int> const& gains, vector<double> const& input) {
        double output = input.at(0);
        if(gains.at(0) == -1)
            output=1/output;
        for(int i=1; i<input.size(); i++){
            if(gains.at(i)==1){
                output*=input[i];
            }else{
                output/=input[i];
            }
        }
        return output;
    }

    shared_ptr<SimpleFunc> Multiply::_inverse(int index) {
        return make_shared<MathPack::Divide>();
    }

    SimpleFunc::FuncTypes Multiply::type(){
        return FuncTypes::tMult;
    }

    string Multiply::getName(int i) {
        if(i==1){
            return "*";
        }else{
            return "/";
        }
    }

    node_container<Node> Multiply::simplify(node_container<FuncNode> input) {
        node_container<Node> output = nullptr;
        auto &inps=input->getInputs();
        auto &gains=input->getGain();
        int len=inps.size();

        //simplify inputs
        for(int i=0;i<inps.size();i++){
            auto &uz = inps[i];
            if(uz->type() == FunctionNode){
                auto rp = cast_node<FuncNode>(uz);
                input->setInput(rp->simplify(), i);
            }
        }

        //gathering
        for(int i=0; i<len; i++){
            auto &uz = inps[i];
            if(uz->type() == FunctionNode){
                auto fuz = cast_node<FuncNode>(uz);
                if(fuz->getFuncType() == FuncTypes::tMult){
                    len += fuz->getInputs().size()-1;

                    for(size_t j=0; j<fuz->getRank(); j++){
                        input->addOperand(fuz->getInputs()[j], fuz->getGain()[j]*gains.at(i));
                        // input was moved; prevent deallocation
                        fuz->setInput(nullptr, j);
                    }

                    // input was moved up one level
                    input->removeInp(i);

                    i--;
                }
            }
        }

        if(inps.size()==1){
            auto ret = inps[0]->copy();
            clear_node(input);
            if(gains.at(0)==1){
                return ret;
            }else{
                return make_node<FuncNode>("inv", ret);
            }
        }

        // decrease consts
        size_t numOfConsts=0;
        vector<double> inputs(input->getRank());
        vector<int> indxs(input->getRank());
        for(int i=0; i<input->getRank(); i++){
            if(inps[i]->type() == ConstNode){
                auto tmpVal = cast_node<Const>(inps[i])->getValue();
                inputs[numOfConsts] = tmpVal;
                indxs[numOfConsts] = i;
                numOfConsts++;
                if(tmpVal == 0.0 && gains[i]==1){
                    clear_node(input);
                    return make_node<Const>(0.0);
                }
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
        }else if(numOfConsts==1){
            if(cast_node<Const>(inps.at(indxs[0]))->getValue() == 1.0){
                input->removeInp(indxs[0]);
            }

            if(inps.size()>1)
                output=input;
            else{
                // removed const, and only one arg left
                if(gains.at(0)==1){
                    output = inps.at(0)->copy();
                    clear_node(input);
                }else{
                    output = make_node<FuncNode>("inv", inps.at(0)->copy());
                    clear_node(input);
                }
            }
        }else{
            double val=1;
            for(size_t i=0; i<numOfConsts; i++){
                val *= pow(inputs[i], gains.at(indxs[i]));
            }
            if(val != 1.0){
                input->addOperand(make_node<Const>(val), 1);
            }
            for(size_t i=numOfConsts; i-->0;){
                input->removeInp(indxs[i]);
            }
            output=input;
        }
        return output;

//        //expand k*(... + ... +)
//        expand:
//        if(output instanceof FuncNode){
//            FuncNode test=(FuncNode)output;
//            if(test.getFuncName().equals("*")){
//                List<Node> testInps=test.getInputs();
//                for(int i=0;i<testInps.size();i++){
//                    Node uz=testInps.get(i);
//                    if(uz instanceof FuncNode){
//                        if(((FuncNode)uz).getFuncName().equals("+")&&test.getGain().get(i)==1){
//                            // create
//                            List<Node> newInps=((FuncNode)uz).getInputs();
//                            List<Node> outInps=new ArrayList();
//                            List<Integer> outGai=new ArrayList();
//                            for(int j=0;j<newInps.size();j++){
//                                List<Node> inp=new ArrayList();
//                                List<Integer> gai=new ArrayList();
//                                for(int k=0;k<testInps.size();k++){
//                                    if(k!=i){
//                                        inp.add(testInps.get(k).copy());
//                                        gai.add(test.getGain().get(k));
//                                    }else{
//                                        inp.add(newInps.get(j).copy());
//                                        gai.add(1);
//                                    }
//                                }
//                                outInps.add(new FuncNode("*", inp, gai));
//                                outGai.add(((FuncNode) uz).getGain().get(j));
////                                newInps.set(j, new FuncNode("*", inp, gai));
//                            }
////                            output=((FuncNode) uz).simplify();
//                            output=new FuncNode("+",outInps,outGai);
//                            break expand;
//                        }
//                    }
//                }
//            }
//        }
    }

    node_container<Node> Multiply::expand(node_container<FuncNode> input) {
        auto &inps = input->getInputs();
        auto &gains = input->getGain();

        //expand inputs
        for (size_t i = 0; i < inps.size(); i++) {
            if (inps[i]->type() == FunctionNode) {
                auto rp = cast_node<FuncNode>(inps[i]);
                input->setInput(rp->expand(), i);
            }
        }

        //expand k*(... + ... +)=
        for(int i=0; i<input->getRank(); i++){
            if(inps[i]->type() == FunctionNode){
                auto rp = cast_node<FuncNode>(inps[i]);
                if(rp->getFuncType()==FuncTypes::tSum && gains.at(i)==1){
                    // create k*s1 + k*s2 + ... k*sn
                    vector<node_container<Node>> outInps;
                    vector<int> outGai;
                    for(int j=0; j<rp->getInputs().size(); j++){
                        vector<node_container<Node>> inp;
                        vector<int> gai;
                        for(int k=0; k<inps.size(); k++){
                            if(k != i){
                                // etc
                                inp.push_back(inps.at(k)->copy());
                                gai.push_back(gains.at(k));
                            }else{
                                // sum function
                                inp.push_back(rp->getInputs().at(j)->copy());
                                gai.push_back(gains.at(i));
                            }
                        }
                        outInps.push_back(make_node<FuncNode>("*", inp, gai));
                        outGai.push_back(rp->getGain().at(j));
                    }
                    clear_node(input);
                    return make_node<FuncNode>("+", outInps, outGai);
                }
                // TODO gain == -1 ??? simply divide everything
            }
        }
        
        return input;
    }

    vector<int> Multiply::getRequiredIndexes() {
        return {-1};
    }

    /**
     * d.a*b+a*d.b
     * @param root Node that call differ method
     * @param varName
     */
    node_container<Node> Multiply::differ(node_container<FuncNode> root, string const& varName) {
        auto &inps = root->getInputs();
        auto &gains = root->getGain();
        node_container<Node> a, b;

        if(gains.at(0) == -1){
            a = make_node<FuncNode>("inv", inps.at(0)->copy());
        }else{
            a = inps.at(0)->copy();
        }

        // TODO size == 1 ???
        if(inps.size() == 2){
            if(gains.at(1) == -1){
                b = make_node<FuncNode>("inv", inps[1]->copy());
            }else{
                b = inps[1]->copy();
            }
        }else{
            vector<node_container<Node>> sub_i;
            vector<int> sub_g;

            for (size_t i=1; root->getRank(); i++){
                sub_i.push_back(inps[i]->copy());
                sub_g.push_back(gains[i]);
            }

            b = make_node<FuncNode>("*", sub_i, sub_g);
        }

        auto left=make_node<FuncNode>("*", a->differ(varName), b);
        auto right=make_node<FuncNode>("*", a, b->differ(varName));
        auto out=make_node<FuncNode>("+", left, right);
        return out->simplify();
    }
}

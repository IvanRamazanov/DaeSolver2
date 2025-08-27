#include "../MathFunction.h"

namespace MathPack{

    /**
     * @param gain2 - optional gain for k
     */
    int _combineInputs(node_container<FuncNode> root, node_container<Node> varNode, size_t inp1, size_t inp2, double k, int gain2=1){
        if(k == 0){
            root->removeInp(max(inp1, inp2));
            root->removeInp(min(inp1, inp2));

            return -2;
        }else{
            vector<node_container<Node>> inps = {make_node<Const>(k),
                                                varNode->copy()};
            vector<int> gns = {1, gain2};
            root->addOperand(make_node<FuncNode>("*", inps, gns),
                            1);

            root->removeInp(max(inp1, inp2));
            root->removeInp(min(inp1, inp2));

            return -1;
        }
    }


    double Summa::Evaluate(vector<int> const& gains, vector<double> const& input) {
        double output=0;
        for(int i=0; i<input.size(); i++){
            output += gains[i] * input[i];
        }
        return output;
    }

    SimpleFunc::FuncTypes Summa::type(){
        return FuncTypes::tSum;
    }

    shared_ptr<SimpleFunc> Summa::_inverse(int index) {
        return make_shared<Subtraction>();
    }

    node_container<Node> Summa::simplify(node_container<FuncNode> input) {
        node_container<Node> output = nullptr;
        auto &inps = input->getInputs();
        auto &gains = input->getGain();
        int len = inps.size();

        //simplify inputs
        for(int i=0; i<inps.size(); i++){
            if(inps[i]->type() == FunctionNode){
                auto rp = cast_node<FuncNode>(inps[i]);
                input->setInput(rp->simplify(), i);
            }
        }

        //gathering
        for(int i=0; i<len; i++){
            if(inps[i]->type() == FunctionNode){
                auto fuz = cast_node<FuncNode>(inps[i]);
                if(fuz->getFuncType() == FuncTypes::tSum){
                    len += fuz->getInputs().size()-1;
                    for (size_t j=0; j<fuz->getRank(); j++){
                        input->addOperand(fuz->getInputs()[j], fuz->getGain()[j]*gains.at(i));
                        // input was moved; prevent deallocation
                        fuz->setInput(nullptr, j);
                    }
                    input->removeInp(i);
                    i--;
                }
            }
        }

        //collecting
        for(int i=0; i<len; i++){
            if(inps[i]->type() == FunctionNode){
                auto fuz = cast_node<FuncNode>(inps[i]);
                if(fuz->getFuncType() == FuncTypes::tMult){ //mb N*f()*g()+M*f()*g()=(N+M)*f()*g() !!!!!
                    if(fuz->getInputs().size() == 2){
                        if((fuz->getInputs()[0]->type() == ConstNode)^(fuz->getInputs()[1]->type() == ConstNode)){
                            auto node_0 = fuz->getInputs()[0],
                                node_1 = fuz->getInputs()[1];

                            string var;
                            double mul;
                            int invgan;
                            node_container<Node> link = nullptr;
                            if(node_0->type() == ConstNode){
                                var = node_1->toString();
                                mul = pow(cast_node<Const>(node_0)->getValue(), fuz->getGain().at(0));
                                invgan = fuz->getGain().at(1);
                                link = node_1;
                            }else{
                                var = node_0->toString();
                                mul = pow(cast_node<Const>(node_1)->getValue(), fuz->getGain().at(1));
                                invgan = fuz->getGain().at(0);
                                link = node_0;
                            }
                            for(int j=i+1; j<len; j++){
                                if(inps[j]->type() == FunctionNode){
                                    auto another = cast_node<FuncNode>(inps[j]);
                                    if(another->getFuncType() == FuncTypes::tMult){
                                        if(another->getInputs().size() == 2){
                                            node_0 = another->getInputs()[0];
                                            node_1 = another->getInputs()[1];
                                            if((node_0->type() == ConstNode)
                                                    ^(node_1->type() == ConstNode)){
                                                string avar;
                                                double amul;
                                                int agan;
                                                if(node_0->type() == ConstNode){
                                                    avar = node_1->toString();
                                                    amul = pow(cast_node<Const>(node_0)->getValue(), another->getGain().at(0));
                                                    agan = another->getGain().at(1);
                                                }else{
                                                    avar = node_0->toString();
                                                    amul = pow(cast_node<Const>(node_1)->getValue(), another->getGain().at(1));
                                                    agan = another->getGain().at(0);
                                                }
                                                if(agan == invgan && var == avar){
                                                    len += _combineInputs(input, link, i, j,
                                                                        gains.at(i)*mul + gains.at(j)*amul,
                                                                        agan);
                                                    i--;
                                                    break;
                                                }
                                            }
                                        }
                                    }else{
                                        string avar=inps.at(j)->toString();
                                        if(invgan==1 && avar == var){
                                            len += _combineInputs(input, inps[j], i, j,
                                                                    gains.at(j) + gains.at(i)*mul);
                                            i--;
                                            break;
                                        }
                                    }
                                }else{
                                    string avar=inps.at(j)->toString();
                                    if(invgan==1 && avar == var){
                                        len += _combineInputs(input, inps[j], i, j,
                                                            gains.at(j) + gains.at(i)*mul);
                                        i--;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }else{
                    string var=inps.at(i)->toString();
                    int gan=gains.at(i);
                    for(int j=i+1;j<len;j++){
                        if(inps.at(j)->type() == FunctionNode){
                            auto another = cast_node<FuncNode>(inps.at(j));
                            if(another->getFuncName() == "*"){
                                if(another->getInputs().size()==2){
                                    auto &node_0 = another->getInputs()[0],
                                        &node_1 = another->getInputs()[1];
                                    if((node_0->type() == ConstNode)^
                                            (node_1->type() == ConstNode)){
                                        string avar;
                                        double mul;
                                        int agan;
                                        if(node_0->type() == ConstNode){
                                            avar=node_1->toString();
                                            mul = cast_node<Const>(node_0)->getValue();
                                            agan=another->getGain().at(1);
                                        }else{
                                            avar=node_0->toString();
                                            mul = cast_node<Const>(node_1)->getValue();
                                            agan=another->getGain().at(0);
                                        }
                                        if(agan==1 && var == avar){
                                            len += _combineInputs(input, inps[i], i, j,
                                                            gan+gains.at(j)*mul);
                                            i--;
                                            break;
                                        }
                                    }
                                }
                            }else{
                                if(another->toString() == var){
                                    len += _combineInputs(input, inps[i], i, j,
                                                            gan+gains.at(j));
                                    i--;
                                    break;
                                }
                            }
                        }else{
                            if(inps.at(j)->toString() == var){
                                len += _combineInputs(input, inps[i], i, j,
                                                            gan+gains.at(j));
                                i--;
                                break;
                            }
                        }
                    }
                }
            }
        }
        // collect vars
        for(int i=0; i<len; i++){
            if(inps.at(i)->type() != VariableNode)
                continue;

            // inps[i] is a variable
            string var = inps.at(i)->toString();
            int gan = gains.at(i);
            for(int j=i+1; j<len; j++){
                if(inps.at(j)->type() == FunctionNode){
                    auto another = cast_node<FuncNode>(inps.at(j));
                    if(another->getFuncType() == FuncTypes::tMult){
                        if(another->getInputs().size()==2){
                            auto node_0 = another->getInputs()[0],
                                node_1 = another->getInputs()[1];

                            if((node_0->type() == ConstNode)^
                                    (node_1->type() == ConstNode))
                            {
                                string avar;
                                double mul;
                                int agan;
                                if(node_0->type() == ConstNode){
                                    avar = node_1->toString();
                                    mul = cast_node<Const>(node_0)->getValue();
                                    agan = another->getGain().at(1);
                                }else{
                                    avar = node_0->toString();
                                    mul = cast_node<Const>(node_1)->getValue();
                                    agan = another->getGain().at(0);
                                }
                                if(agan==1 && var == avar){
                                    len += _combineInputs(input, inps[i], i, j,
                                                        gan+gains.at(j)*mul);
                                    i--;
                                    break;
                                }
                            }
                        }
                    }else{
                        if(another->toString() == var){
                            len += _combineInputs(input, inps[i], i, j,
                                                gan+gains.at(j));
                            i--;
                            break;
                        }
                    }
                }else{
                    if(inps.at(j)->toString() == var){
                        len += _combineInputs(input, inps[i], i, j,
                                            gan+gains.at(j));
                        i--;
                        break;
                    }
                }
            }
        }

        // if after all the cleaning this node self "destruct", return const 0 (sum of nothing is 0)
        if(inps.empty()){
            clear_node(input);
            return make_node<Const>(0);
        }

        //convert -1*k=-k
        for(size_t i=0; i<input->getRank(); i++){
            if(inps[i]->type() == FunctionNode){
                auto func_at_i = cast_node<FuncNode>(inps[i]);
                if(func_at_i->getFuncType() == FuncTypes::tMult){
                    auto &func_at_i_inps = func_at_i->getInputs();
                    for(size_t j=0; j<func_at_i->getRank(); j++){
                        if(func_at_i_inps[j]->type() == ConstNode){
                            if(cast_node<Const>(func_at_i_inps[j])->getValue() < 0.0){
                                input->setGain(-1*gains[i], i);

                                // replace j-th input of i-th function
                                auto newInp = make_node<Const>(cast_node<Const>(func_at_i_inps[j])->getValue()*-1.0);
                                clear_node(func_at_i_inps[j]);
                                func_at_i->setInput(newInp, j);

                                input->setInput(func_at_i->simplify() ,i);
                                break;
                            }
                        }
                    }
                }
            }
        }

        // decrease consts
        size_t numOfConsts=0;
        vector<double> inputs(inps.size());
        vector<int> indxs(inps.size());
        for(int i=0; i<inps.size(); i++){
            auto &uz = inps[i];
            if(uz->type() == ConstNode){
                inputs[numOfConsts] = cast_node<Const>(uz)->getValue();
                indxs[numOfConsts]=i;
                numOfConsts++;
            }
        }
        if(numOfConsts==inps.size()){
            output = make_node<Const>(Evaluate(gains, inputs));
            clear_node(input);
        }else if(inps.size()==1){
            if(gains.at(0) == 1){
                output = inps[0]->copy();
                clear_node(input);
            }else{
                output = input;
            }
        }else if(numOfConsts==1){
            if(cast_node<Const>(inps.at(indxs[0]))->getValue() == 0.0){
                // ... + 0.0; remove
                input->removeInp(indxs[0]);
            }
            output = input;
        }else{
            double val=0;
            for(size_t i=0; i<numOfConsts; i++){
                val += gains.at(indxs[i])*inputs[i];
            }
            if(val != 0.0){
                input->addOperand(make_node<Const>(val), 1);
            }
            for(size_t i=numOfConsts; i-->0;){
                input->removeInp(indxs[i]);
            }
            output = input;
        }

        return output;
    }

    node_container<Node> Summa::expand(node_container<FuncNode> input) {
        auto &inps = input->getInputs();
        auto &gains = input->getGain();

        //expand inputs
        for (size_t i = 0; i < input->getRank(); i++) {
            if (inps[i]->type() == FunctionNode) {
                auto rp = cast_node<FuncNode>(inps[i]);
                input->setInput(rp->expand(), i);
            }
        }

        return input;
    }

    // SimpleFunc* Summa::_copy() {
    //     return new Summa();
    // }

    string Summa::getName(int i) {
        if(i==1){
            return "+";
        }else{
            return "-";
        }
    }

    vector<int> Summa::getRequiredIndexes() {
        vector<int> out = {-1};
        return out;
    }

    node_container<Node> Summa::differ(node_container<FuncNode> root, string const& varName) {
        vector<node_container<Node>> inp;
        for(size_t i=0; i<root->getInputs().size(); i++){
            inp.push_back(root->getInputs()[i]->differ(varName));
        }
        auto out = make_node<FuncNode>("+", inp, root->getGain());
        return out->simplify();
    }
}

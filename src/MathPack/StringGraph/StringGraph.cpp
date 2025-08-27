#include "../StringGraph.h"
#include "../Vectors.h"

namespace MathPack{

    bool onlySimpleFuncs(vector<node_container<FuncNode>> const& inp){
        bool flag=true;
        for(auto& uz:inp){
            if(uz->getFuncType() != SimpleFunc::tSum && uz->getFuncType() != SimpleFunc::tSubtract){
                flag=false;
            }
        }
        return flag;
    }

    StringGraph::StringGraph(string const& rawString){
        if (rawString.empty()){
            root = make_node<Const>(0.0);
            return;
        }

        root = createNode(rawString);
        if (root->type() == FunctionNode){
            simplify();
        }else if(root->type() == VariableNode){
            auto vp = cast_node<Variable>(root);
            if (!MathPack::contains(variables, vp->getName()))
                variables.push_back(vp->getName());
        }
    }

    StringGraph::StringGraph(StringGraph const& input){
        this->variables = input.variables;
        this->root = input.root->copy();
    }

    StringGraph& StringGraph::operator=(StringGraph const& other){
        this->variables = other.variables;
        this->root = other.root->copy();
        return *this;
    }

    StringGraph::StringGraph(double val){
        root = make_node<Const>(val);
    }

    StringGraph::StringGraph(WorkSpace::Variable *var, string const& customVarName){
        auto r = make_node<Variable>(customVarName.empty() ? var->getName() : customVarName, var);
        r->setWsLink(var);
        root = r;
    }

    StringGraph::StringGraph(node_container<Node> uz){
        root = uz;

        simplify();
    }

    StringGraph::~StringGraph(){
        clear_node(root);
    }

    shared_ptr<StringGraph> StringGraph::getDiffer(string const& varName){
        auto out = make_shared<StringGraph>(root->differ(varName));
        out->simplify();

        return out;
    }

    shared_ptr<StringGraph> StringGraph::getFullTimeDiffer(vector<string> const& glabalVarNames,
                                                    //vector<shared_ptr<WorkSpace::Variable>> const& varList,
                                                    vector<shared_ptr<WorkSpace::DifferentialVar>> const& diffList)
    {
        vector<string> vars = getVariableNames();

        if(vars.empty()){
            return make_shared<StringGraph>(0.0);
        }
        else if(vars.size()==1 && vars[0]==WorkSpace::TIME_VAR_NAME) {
            return getDiffer(WorkSpace::TIME_VAR_NAME);
        }
        
        // try to find diff in 'workspace'
        size_t varIdx = -1;
        for(auto i=glabalVarNames.size(); i-->0;){
            if (glabalVarNames[i] == vars[0]){
                varIdx = i;
                break;
            }
        }
        if (varIdx == size_t(-1)){
            throw runtime_error("Can't find differential of " + vars[0]+" in workspace!");
        }

        auto out = getDiffer(vars[0]);
        auto varPath = XmlParser::split(glabalVarNames[varIdx], ElementBase::ELEM_PATH_DELIM);
        varPath.back() = diffList[varIdx]->getName();
        auto diff0 = make_node<Variable>(XmlParser::join(varPath, ElementBase::ELEM_PATH_DELIM),
                                            diffList[varIdx].get());
        out->multiply(make_shared<StringGraph>(diff0));

        for(size_t i=1; i<vars.size(); i++){
            // try to find diff in 'workspace'
            varIdx = -1;
            for(auto j=glabalVarNames.size(); j-->0;){
                if (glabalVarNames[j] == vars[i]){
                    varIdx = j;
                    break;
                }
            }
            if (varIdx == size_t(-1)){
                throw runtime_error("Can't find differential of " + vars[i]+" in workspace!");
            }

            varPath = XmlParser::split(glabalVarNames[varIdx], ElementBase::ELEM_PATH_DELIM);
            varPath.back() = diffList[varIdx]->getName();
            auto diff1 = make_node<Variable>(XmlParser::join(varPath, ElementBase::ELEM_PATH_DELIM),
                                                diffList[varIdx].get());

            auto temp = getDiffer(vars[i]);
            temp->multiply(make_shared<StringGraph>(diff1));
            out->add(temp);
        }

        return out;
    }

    void StringGraph::linkVariableToWorkSpace(string const& name, WorkSpace::Variable *link){
        if (root->type() == VariableNode){
            auto dp = cast_node<Variable>(root);
            if (dp->getName() == name)
                dp->setWsLink(link);
        }else if(root->type() == FunctionNode){
            vector<node_container<FuncNode>> next_nodes = {cast_node<FuncNode>(root)};
            while(!next_nodes.empty()){
                vector<node_container<FuncNode>> tmpVec;
                for (auto& fn:next_nodes){
                    for(auto& n:fn->getInputs()){
                        if (n->type() == VariableNode){
                            auto tmp = cast_node<Variable>(n);
                            if(tmp->getName() == name){
                                tmp->setWsLink(link);
                            }
                        }else if(n->type() == FunctionNode){
                            tmpVec.push_back(cast_node<FuncNode>(n));
                        }
                    }
                }
                next_nodes = tmpVec;
            } 
        }
    }

    void StringGraph::simplify(){
        if(root->type() == FunctionNode){
            auto rp = cast_node<FuncNode>(root);
            root = rp->simplify();
        }
    }

    void StringGraph::expand(){
        if(root->type() == FunctionNode){
            auto rp = cast_node<FuncNode>(root);
            root = rp->expand();
        }
    }

    string StringGraph::toString(){
        return root->toString();
    }

    /**
     * Check if this graph contains only X.i, d.X.i, time.
     * @return
     */
    bool StringGraph::isInvariant(){
        getVariableNames(); // to refresh the list
        bool flag=true;
        for(auto& name:variables){
            if(name != "time"){
                flag=false;
                break;
            }
        }
        return flag;
    }

    int StringGraph::canGet(string const& varType, int numOfVars){
        int varNum=1;
        for(;varNum<=numOfVars;varNum++){
            if(root->numOfContains(varType + to_string(varNum)) == 1){
                return varNum-1;
            }
        }
        return -1;
    }

    bool StringGraph::canGet(string const& varName){
        bool outFlag=false;
        auto numCont = root->numOfContains(varName);
        if(numCont == 1){
            if(onlySimpleFuncs(findPath(varName)->getFuncs()))
                outFlag=true;
        }
        return outFlag;
    }

    /**
     * 
     */
    void StringGraph::add(shared_ptr<StringGraph> graph){
        if(graph->root->type() == ConstNode){
            if(cast_node<Const>(graph->root)->getValue() == 0){
                return;
            }
        }
        if(root->type() == ConstNode){
            if(cast_node<Const>(root)->getValue() == 0){
                clear_node(root);
                root = graph->root->copy();
                return;
            }
        }

        root = make_node<FuncNode>("+", root, graph->root->copy());
        
        simplify();
    }

    /**
     * Returns this-graph
     * @param graph
     */
    void StringGraph::sub(shared_ptr<StringGraph> graph){
        if(graph->root->type() == ConstNode){
            if(cast_node<Const>(graph->root)->getValue() == 0){
                return;
            }
        }

        if(root->type() == ConstNode){
            if(cast_node<Const>(root)->getValue() == 0){
                clear_node(root);
                root = graph->root->copy();
                multiply(-1);
                return;
            }
        }

        vector<int> gains{1, -1};
        vector<node_container<Node>> inps;
        inps.push_back(root);
        inps.push_back(graph->root->copy());

        root = make_node<FuncNode>("+", inps, gains);

        simplify();
    }

    /**
     * this = this-inp
     * @param inp
     */
    // void StringGraph::sub(node_container<Node> inp){
    //     if(inp->type() == ConstNode){
    //         if(cast_node<Const>(inp)->getValue() == 0){
    //             return;
    //         }
    //     }

    //     vector<int> gains;
    //     gains.push_back(1);
    //     gains.push_back(-1);
    //     vector<node_container<Node>> inps;
    //     inps.push_back(root);
    //     inps.push_back(inp);

    //     root = make_node<FuncNode>("+", inps, gains);

    //     simplify();
    // }

    /**
     * Note: takes ownership of graph's root
     */ 
    void StringGraph::multiply(shared_ptr<StringGraph> graph){
        root = make_node<FuncNode>("*", root, graph->root->copy());

        simplify();
    }

    /**
     * unused
     */
    // void StringGraph::divide(shared_ptr<StringGraph> graph){
    //     vector<node_container<Node>> inps;
    //     inps.push_back(root);
    //     inps.push_back(graph->root);
    //     vector<int> gains;
    //     gains.push_back(1);
    //     gains.push_back(-1);

    //     root = make_node<FuncNode>("*", inps, gains);

    //     simplify();
    // }

    void StringGraph::multiply(double value){
        if(value==1)
            return;

        auto mul = make_node<Const>(value);

        root = make_node<FuncNode>("*", root, mul);

        simplify();
    }

    void StringGraph::replaceVariable(string const& name, shared_ptr<StringGraph> replacement){
        if(root->type() == VariableNode){
            if(cast_node<Variable>(root)->getName() == name){
                clear_node(root);
                root = replacement->root->copy();
            }
        }else root->replaceVar(name, replacement->root->copy());

        simplify();
    }

    void StringGraph::renameVariable(string const& name, string const& newName){
        root->setName(name, newName);
    }

    void StringGraph::replaceWithConst(string const& name, double value){
        if(root->type() == VariableNode){
            if(cast_node<Variable>(root)->getName() == name){
                clear_node(root);
                root = make_node<Const>(value);
            }
        }else root->replaceVar(name, make_node<Const>(value));
    }

    bool StringGraph::contains(string const& var){
        return root->contains(var);
    }

    int StringGraph::numOfContains(string const& var){
        return root->numOfContains(var);
    }

    /**
     * Case: k*X=f(X,u)
     * @param name
     * @param k
     */
    void StringGraph::getVariable(string const& name, int k){
        simplify();

        auto lft = make_node<FuncNode>("+", make_node<Const>(k));
        auto path = findPath(name);

        if(path == nullptr){
            throw runtime_error("No variable: " + name + " in: " + toString());
        }

        for(size_t i=0; i<path->length(); i++){
            auto fu = path->getNode(i);
            if(fu->getFuncType() == SimpleFunc::tMult){
                // invert the path
                size_t ind = path->numberOfOperand.at(i);
                vector<node_container<Node>> inps;
                vector<int> gains;
                for(size_t m=0; m < fu->getInputs().size(); m++){
                    if(m != ind){
                        inps.push_back(fu->getInputs()[m]->copy());
                        gains.push_back(fu->getGain()[m]);
                    }else{
                        inps.push_back(make_node<Const>(-1));
                        gains.push_back(1);
                    }
                }
                auto tmp = make_node<FuncNode>("*", inps, gains);

                lft->addOperand(tmp, 1);
            }

        }
        root->replaceVar(name, make_node<Const>(0));
        root = make_node<FuncNode>("*", root, lft);
        cast_node<FuncNode>(root)->setGain(-1, 1);

        simplify();
    }

    void StringGraph::getVariable(string const& name, shared_ptr<StringGraph> left){
        auto path=findPath(name);
        if(path == nullptr){
            throw runtime_error("No variable: "+name+" in: "+toString());
        }else if(path->length()==0){
            // path is not null, but len == 0 means root is Variable with matching name
            // therefore, name = left
            clear_node(root);
            root = left->getRoot()->copy();
        }else{
            sub(left);
            getVariable(name);
            // int len=path->length();
            // for(size_t i=0; i<path->length(); i++){
            //     auto fu = path->getNode(i);
            //     size_t j = i+1;
            //     node_container<Node> nextfu;
            //     if(j == len){
            //         nextfu = left->root->copy();
            //     }else{
            //         nextfu = path->getNode(j)->copy();
            //     }
            //     fu->invertNode(nextfu, path->numberOfOperand.at(i));
            // }

            // //root = newRoot;
            // // ^ unnecessary, since first node in the path is root 

            // simplify();
        }
    }

    void StringGraph::getVariable(string const& name){
        auto path = findPath(name);
        if(path == nullptr){
            throw runtime_error("No variable: "+name+" in: "+toString());
        }

        if(path->length() == 0){
            // double check that root is var with name == name
            if(root->type()==VariableNode){
                auto vp = cast_node<Variable>(root);
                if (vp->getName() == name){
                    clear_node(root);
                    root = make_node<Const>(0);
                    return;
                }
            }
            throw runtime_error("StringGraph::getVariable - root is not a function!");
        }

        auto newRoot = path->getNode(0);
        size_t len = path->length();
        for(size_t i=0; i<len; i++){
            auto fu = path->getNode(i);
            size_t j = i+1;
            node_container<Node> nextfu;
            if(j == len){
                nextfu = make_node<Const>(0);
            }else{
                nextfu = path->getNode(j);
            }
            fu->invertNode(nextfu, path->numberOfOperand.at(i));
        }

        root = newRoot;
        simplify();
    }

    double StringGraph::evaluate() const {
        return(root->getValue());
    }

    vector<string> StringGraph::getVariableNames(){
        variables.clear();
        root->getVariables(variables);
        return variables;
    }

    vector<node_container<Variable>> StringGraph::getVariables(){
        vector<node_container<Variable>> ret;
        if (root->type() == VariableNode){
            ret.push_back(cast_node<Variable>(root));
        }else if(root->type() == FunctionNode){
            vector<node_container<FuncNode>> next_nodes = {cast_node<FuncNode>(root)};
            while(!next_nodes.empty()){
                vector<node_container<FuncNode>> tmpVec;
                for (auto& fn:next_nodes){
                    for(auto& n:fn->getInputs()){
                        if (n->type() == VariableNode){
                            auto tmp = cast_node<Variable>(n);
                            bool hasVar = false;
                            for (auto& r:ret){
                                if(tmp->getName() == r->getName()){
                                    hasVar = true;
                                    break;
                                }
                            }
                            if(!hasVar){
                                ret.push_back(tmp);
                            }
                        }else if(n->type() == FunctionNode){
                            tmpVec.push_back(cast_node<FuncNode>(n));
                        }
                    }
                }
                next_nodes = tmpVec;
            } 
        }

        return ret;
    }

    shared_ptr<Path> StringGraph::findPath(string const& varName){
        //levels=0;
        auto path = make_shared<Path>();
        if(root->contains(path, varName)){
            return path;
        }else return nullptr;
    }

    node_container<Node> StringGraph::getRoot(){
        return root;
    }
}

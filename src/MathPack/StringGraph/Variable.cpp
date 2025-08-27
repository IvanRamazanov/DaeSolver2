#include "../StringGraph.h"
#include "../Vectors.h"

namespace MathPack{
    /**
     * Variable class definition
     */
    Variable::Variable(string const& name){
        this->name=name;
    }

    Variable::Variable(string const& name, WorkSpace::Variable *workSpaceLink): 
        Variable(name)
    {
        this->workSpaceLink = workSpaceLink;
        this->value = workSpaceLink->getRef();
    }

    const string& Variable::getName(){
        return name;
    }

    void Variable::setName(string const& name, string const& newName){
        if (this->name == name)
            this->name = newName;
    }

    void Variable::setVariableLink(string const& name, double *wslink){
        if(this->name == name){
            value=wslink;
        }
    }

    double Variable::getValue(){
        return *value;
    }

    void Variable::replaceVar(string const& name, node_container<Node> const& replacement) {}
    
    bool Variable::contains(shared_ptr<Path> path, string const& name) {
        return this->name == name;
    }

    bool Variable::contains(string const& name) {
        return this->name == name;
    }

    node_container<Node> Variable::differ(string const& varName) {
        if(name == varName)
            return make_node<Const>(1.0);
        else
            return make_node<Const>(0.0);
    }

    int Variable::numOfContains(string const& varName) {
        if(name == varName){
            return 1;
        }else{
            return 0;
        }
    }

    void Variable::getVariables(vector<string>& inp) {
        if (!MathPack::contains(inp, name))
            inp.push_back(name);
    }

    node_container<Node> Variable::copy() {
        // TODO: validate for C++
        auto n = make_node<Variable>(getName());
        n->value = this->value;
        n->workSpaceLink = this->workSpaceLink;
        return n;
    }

    string Variable::toString() {
        // TODO replace whitespaces with something / erase (?)
        return getName();
    }

    GraphNodeType Variable::type() {
        return GraphNodeType::VariableNode;
    }

    WorkSpace::Variable* Variable::getWsLink(){
        return workSpaceLink;
    }
            
    void Variable::setWsLink(WorkSpace::Variable* wsVar){
        workSpaceLink = wsVar;
        value = wsVar->getRef();
    }

    void Variable::debugInfo(int depth) {
        qDebug().noquote() << QString("    ").repeated(depth) << "Var: " << name << " addr:" << (size_t)this << " val_addr:" << (size_t)value;
    }
}

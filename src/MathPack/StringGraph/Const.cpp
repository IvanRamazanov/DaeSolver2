#include "../StringGraph.h"

namespace MathPack{
    /**
     * Const class definition
     */
    Const::Const(double value){
        this->value = value;
    }

    Const::Const(string const& str){
        this->value = stod(str);
    }


    double Const::getValue(){
        return value;
    }

    void Const::setValue(double newVal){
        value = newVal;
    }

    void Const::setName(string const& name, string const& newName){}

    node_container<Node> Const::differ(string const& varName){
        return make_node<Const>(0.0);
    }

    void Const::replaceVar(string const& name, node_container<Node> const& replacement){}

    bool Const::contains(shared_ptr<Path> path, string const& varName){
        return false;
    }

    bool Const::contains(string const& varName){
        return false;
    }

    void Const::getVariables(vector<string>& inp){
        // const can't have variables
    }

    void Const::setVariableLink(string const& name, double *link){}

    int Const::numOfContains(string const& varName){
        return 0;
    }

    node_container<Node> Const::copy(){
        return make_node<Const>(value);
    }

    string Const::toString() {
        return to_string(value);
    }

    GraphNodeType Const::type() {
        return GraphNodeType::ConstNode;
    }

    void Const::debugInfo(int depth) {
        qDebug().noquote() << QString("    ").repeated(depth) << "Const: " << value << " addr:" << (size_t)this;
    }
}

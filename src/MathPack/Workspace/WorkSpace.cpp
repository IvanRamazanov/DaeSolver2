/*
 * The MIT License
 *
 * Copyright 2017 Ivan.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "../WorkSpace.h"

using namespace std;

namespace WorkSpace{
    /**
     * Variable class definition
    */
    Variable::Variable(){}

    Variable::Variable(string const& name, double value, bool isConst, WsDataType type): isConst(isConst), dataType(type){
        this->name=name;
        this->value = {value};
        initValue = this->value;
        size = 1;
    }

    Variable::Variable(string const& name, vector<double> const& initVal): isConst(false), dataType(WsDataType::Discrete){
        this->name=name;
        this->value = initVal;
        initValue = initVal;
        size = initVal.size();
    }

    void Variable::setValue(double val){
        value[0] = val;
    }

    void Variable::setValue(vector<double> const& val){
        std::memcpy(value.data(), val.data(), value.size()*sizeof(double));
    }

    double Variable::getValue(){
        return value[0];
    }

    const string& Variable::getName() {
        return name;
    }

    WsTypes Variable::type() {
        if(size==1)
            return WsTypes::Scalar;
        else
            return WsTypes::Vector;
    }

    double* Variable::getRef(){
        return value.data();
    }

    void Variable::init(){
        for(auto i=initValue.size(); i-->0;)
            value[i] = initValue[i];
    }

    void Variable::init(double val){
        initValue = {val};
        size = 1;
        value.resize(size);
        init();
    }

    size_t Variable::getSize(){
        return size;
    }

    void Variable::init(vector<double> const& val){
        initValue = val;
        size = initValue.size();
        value.resize(size);
        init();
    }


    /**
     * WorkSpace class def
     */
    WorkSpace::WorkSpace(){
    }

    double WorkSpace::get(int indx){
        return variableList.at(indx)->getValue();
    }

    bool WorkSpace::isDynamicVariable(Variable *var){
        return var->dataType != Discrete;
    }

    vector<string> WorkSpace::getVarNameList(){
        vector<string> out;
        for(size_t i=0; i < variableList.size(); i++)
            out.push_back(variableList[i]->getName());
        return out;
    }

    string WorkSpace::getName(int i){
        return variableList.at(i)->getName();
    }

    void WorkSpace::add(shared_ptr<Variable> var){
        if(get(var->getName()) == nullptr){
            variableList.push_back(var);
        }else{
            // don't override already existing var
            throw runtime_error("Workspace already contains var: " + var->getName());
        }
    }

    Variable* WorkSpace::add(string const& name, double value, bool isConst, WsDataType type){
        auto newVar = get(name);
        
        if(newVar == nullptr){
            auto newVar = make_shared<Variable>(name, value, isConst, type);
            variableList.push_back(newVar);
            return newVar.get();
        }else{
            // don't override already existing var
            return newVar;
        }
    }

    DifferentialVar* WorkSpace::addDiff(string const& baseName, WsDataType type){
        shared_ptr<Variable> found = nullptr;
        for (auto& v:variableList){
            if (baseName == v->getName()){
                found = v;
                break;
            }
        }
        if (found == nullptr){
            // create new var to link it to the diff
            found = make_shared<Variable>(baseName, 0, false, type);
            variableList.push_back(found);
        }
        // TODO check for duplicates?
        auto ret = make_shared<DifferentialVar>(found, type);
        variableList.push_back(ret);
        return ret.get();
    }

    Variable* WorkSpace::addVector(string const& name, vector<double> const& value){
        // TODO check for duplicates?
        auto ret = make_shared<Variable>(name, value);
        variableList.push_back(ret);
        return ret.get();
    }

    const vector<shared_ptr<Variable>>& WorkSpace::getVarList() const {
        /**
         * @return the varList
         */

        return variableList;
    }

    Variable* WorkSpace::get(string const& name) const {
        for(auto& var:variableList){
            if(var->getName() == name)
                return var.get();
        }
        return nullptr;
    }

    bool WorkSpace::contains(string const& name) const {
        for(auto& var : variableList){
            if(var->getName() == name)
                return true;
        }
        return false;
    }
}

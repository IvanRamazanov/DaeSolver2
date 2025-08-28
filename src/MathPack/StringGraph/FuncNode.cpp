#include "../StringGraph.h"
#include <sstream>
#include "../Vectors.h"

constexpr char OPERATORS[] = {'+', '-', '*', '/'}; //, '^'};
constexpr int PRIORITY[] = {1, 1, 2, 2}; //, 3};
constexpr int MIN_PRIORITY=1;
constexpr int MAX_PRIORITY=4; // why not 3???

namespace MathPack{
    bool isOperator(SimpleFunc::FuncTypes fType){
        return fType == SimpleFunc::tSum || fType == SimpleFunc::tSubtract || \
                fType == SimpleFunc::tMult || fType == SimpleFunc::tDivide;
    }

    bool isOperator(string const& str, int symbolPos){
        char symbol = str.at(symbolPos);
        bool flag = contains(OPERATORS, symbol);

        if(flag){
            if(symbolPos == 0){
                return flag;
            }

            symbol = str.at(symbolPos-1);
            if(symbol=='e' || symbol=='E'){
                if(symbolPos==1){
                    return flag;
                }
                symbol=str.at(symbolPos-2);
                if(isdigit(symbol)){
                    flag=false;
                }
            }
        }
        return flag;
    }

    /**
     * Checks if string value is a mathematical function
     * @param function string
     * @param pos index of '('
     * @return
     */
    bool isFunction(string const& function, int const pos){
        if(pos==0){
            return false;
        }else{
            for(int index=pos-1; index>=0; index--){
                char ch = function.at(index);
                if(isOperator(function, index)){
                    return false;
                }else if(ch != '('){
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * Gets priority
     * @param ch +,-,... etc.
     * @return
     */
    int getOperPriority(char ch){
        int out=-1;
        for(size_t i=0; i < sizeof(OPERATORS)/sizeof(OPERATORS[0]); i++){
            if(OPERATORS[i] == ch){
                out = PRIORITY[i];
                break;
            }
        }
        return out;
    }

    


    FuncNode::~FuncNode(){
        for(auto j=inputs.size(); j-->0;){
            clear_node(inputs[j]);
        }
    }

    /**
     * Removes reducable brackets from the ends of function string: "(...)"
     */
    void stripBrackets(string & function){
        int bracketDepth = 0;
        size_t strLen = function.length(),
                firtBracketPos = -1;
        while (function[0] == '('){
            bool madeChange = false,
                canBeReplaced = true; // in case of "(...)+...-(...)"
            
            for(size_t i=0; i<strLen; i++){
                if(function[i] == '('){
                    bracketDepth++;
                    if(firtBracketPos == size_t(-1))
                        firtBracketPos = i; // store first opening bracket idx 
                }else if(function[i] == ')'){
                    bracketDepth--;
                    if (canBeReplaced){
                        if(bracketDepth == 0 && // has complete brackets "(...)"
                            firtBracketPos == 0 && // opening bracket is at the very start
                            i == (strLen-1)) // reached the end of input string
                        {
                            // remove first and last charater
                            function = function.substr(1, strLen-2);
                            madeChange = true;
                            strLen = function.length();
                        }
                        else if(bracketDepth == 0){
                            canBeReplaced = false;
                        }
                    }
                }
            }

            // if not 0, then errnous function
            if (bracketDepth != 0)
                throw runtime_error("Invalid brackets in function: " + function);

            if (!madeChange)
                break;
        }
    }

    void trimSpaces(string & function){
        if (function.empty())
            return;

        size_t firstNonSpace=0, lastNonSpace=0;
        bool foundStart=false;
        for (size_t i=0; i<function.length(); i++){
            if (function[i] != ' ' && function[i] != '\t'){
                if(!foundStart) {
                    foundStart = true;
                    firstNonSpace = i;
                }
                lastNonSpace = i;
            }
        }

        function = function.substr(firstNonSpace, lastNonSpace-firstNonSpace+1);
    }

    /**
     * Supports only simple input.
     * @param input
     * @return
     */
    bool isDigit(string const& input){
        try{
            size_t pos = 0;
            stod(input, &pos);
            return pos == input.size(); // check if all characters were consumed
        }catch(exception& ex){
            return false;
        }
    }

    GraphNodeType classifyStringGraph(string const& function) {
        if (function.empty())
            throw invalid_argument("Function string is empty!");

        // check if it's a digit
        if(isDigit(function)){
            return GraphNodeType::ConstNode;
        }

        // test if it's a function of variable
        bool isVar = true;
        for(size_t i=0; i<function.length(); i++){
            char ch = function[i];
            if(isOperator(function, i)){
                isVar = false;
                break;
            }else if(ch == '('){
                if(i != 0){
                    if(!isOperator(function, i-1)){   //a lot of brackets ?
                        isVar=false;
                        break;
                    }
                }
            }
        }
            
        return isVar ? GraphNodeType::VariableNode : GraphNodeType::FunctionNode;
    }

    node_container<Node> createNode(string str){
        // remove unnecessary brackets
        stripBrackets(str);
        trimSpaces(str);

        auto nodeType = classifyStringGraph(str);
        switch(nodeType){
            case GraphNodeType::ConstNode:
                return make_node<Const>(str);
            case GraphNodeType::VariableNode:
                return make_node<Variable>(str);
            default:
                return make_node<FuncNode>(str);
            // assert?
        }
    }

    FuncNode::FuncNode(string const& function, vector<node_container<Node>> input, vector<int> gains){
        if (gains.empty()){
            for(auto i=input.size(); i-->0;){
                gains.push_back(1);
            }
        }
        if (input.size() != gains.size())
            throw invalid_argument("FunctionNode: input and gains arguments must have the same size!");

        gain = gains;
        func = MathFunction(function);
        for(auto& uz:input){
            inputs.push_back(uz);
        }
        this->input.resize(inputs.size());
    }

    FuncNode::FuncNode(FuncNode & other){
        this->func = other.func;
        this->gain = other.gain;
        for(size_t i=0;i<other.inputs.size();i++){
            this->inputs.push_back(other.inputs[i]->copy());
        }
        this->input = other.input;
    }

    FuncNode& FuncNode::operator=(FuncNode & other){
        this->func = other.func;
        this->gain = other.gain;
        for(size_t i=0;i<other.inputs.size();i++){
            this->inputs.push_back(other.inputs[i]->copy());
        }
        this->input = other.input;

        return *this;
    }

    FuncNode::FuncNode(string rawString){
        // TODO: strip functions?

        // verify brackets
        int bracketDepth = 0;
        for (char& ch:rawString){
            if (ch == '('){
                bracketDepth++;
            }else if (ch == ')'){
                bracketDepth--;
            }

            if (bracketDepth<0)
                throw invalid_argument("Corrupted brackets in function: "+rawString);
        }
        if (bracketDepth != 0)
            throw runtime_error("Invalid brackets in: "+rawString);

        int operatorPos = -1,
            minPryor = MIN_PRIORITY;
        size_t numOfOperands=0;
        // break down to operands: operand[i]*sign[i] operator operand[i+1]*sign[i+1] operator ...
        for(;minPryor<MAX_PRIORITY; minPryor++){
            vector<string> operands;
            vector<int> signs;
            signs.push_back(1);
            string operandTmp="";
            for(size_t i=0; i<rawString.length(); i++){
                char c = rawString[i];
                if(bracketDepth == 0 && isOperator(rawString, i)){
                    int pry = getOperPriority(c)+bracketDepth*(MAX_PRIORITY);
                    if(minPryor == pry){
                        if(operandTmp.empty()){
                            if(c=='-'){
                                signs[numOfOperands] = -1;
                            }else{
                                // Note: only '-' operator could be unary
                                ostringstream oss;
                                oss << "Invalid function: "<< rawString << endl << "Operator " << rawString[i] << " must have LHS operand!" ;
                                throw runtime_error(oss.str());
                            }
                        }else{
                            // append operands
                            operands.push_back(operandTmp);
                            // append signs
                            if(c=='-' || c=='/'){
                                signs.push_back(-1);
                            }else{
                                signs.push_back(1);
                            }

                            operandTmp.clear();
                        }
                        numOfOperands++;
                        operatorPos = i;

                        continue; // to prevent operator being added to operandTmp
                    }
                }
                operandTmp += c;
                //skobka case
                if(c=='('){
                    bracketDepth++;
                }else if(c==')'){
                    // check if this is a function!
                    bracketDepth--;
                }

            }

            // if found operators
            if(numOfOperands != 0){
                if(!operandTmp.empty()){
                    operands.push_back(operandTmp);
                    // sign???
                }
                
                // create operator function
                if(rawString[operatorPos] == '+' || rawString[operatorPos] == '-'){
                    func = MathFunction("+");   //create uzel
                }else if(rawString[operatorPos] == '*' || rawString[operatorPos] == '/'){
                    func = MathFunction("*");
                // }else if(rawString[operatorPos] == '^'){
                //     if (numOfOperands < 2)
                //         throw runtime_error("Pow operator '^' requires at least 2 operands!\n"+rawString);

                //     string operand(1, rawString[operatorPos]);
                //     func = MathFunction(operand);
                }else{
                    ostringstream oss;
                    oss << "Unexpected function or operand "<<rawString[operatorPos]<<" at "<<operatorPos<<" in "<<rawString;
                    throw runtime_error(oss.str());
                }
                // fill operator function with operands
                for(size_t k=0; k<operands.size(); k++){
                    inputs.push_back(createNode(operands[k]));
                }
                gain = signs;

                input.resize(inputs.size());
                if (input.size() != gain.size())
                    throw invalid_argument("FunctionNode: input and gains arguments must have the same size!");
                return;
            }
        }

        // has no operators; check if it's a function
        for(size_t i=0; i<rawString.length(); i++){
            if (rawString[i] == '('){
                if(isFunction(rawString, i)){
                    // hit a function. Find its end
                    vector<string> argv;
                    string argTmp;
                    
                    bracketDepth++;
                    for (auto j=i+1; j<rawString.length(); j++){
                        if (rawString[j] == '('){
                            bracketDepth++;
                            argTmp += rawString[j];
                        }else if (rawString[j] == ')'){
                            bracketDepth--;
                            if(bracketDepth == 0){
                                // found the end
                                if (j != rawString.length()-1){
                                    ostringstream oss;
                                    oss << "Badly formatted function body:"<<rawString<<"\nExpected ')' at the end. Got:" << rawString[j];
                                    throw runtime_error(oss.str());
                                }

                                if(!argTmp.empty())
                                    argv.push_back(argTmp);

                                string funcName = rawString.substr(0, i);

                                this->func = MathFunction(funcName);
                                size_t argc = this->func.getRank();
                                

                                // verify arguments size
                                if (argc != argv.size()){
                                    ostringstream oss;
                                    oss << "Function " << funcName << " expects " << argc << " inputs; got:" << argv.size();
                                    oss << endl << "\t" << rawString.substr(i+1, j-i-1);
                                    throw runtime_error(oss.str());
                                }

                                for (auto& arg:argv){
                                    inputs.push_back(createNode(arg));
                                    gain.push_back(1);
                                }

                                input.resize(inputs.size());
                                if (input.size() != gain.size())
                                    throw invalid_argument("FunctionNode: input and gains arguments must have the same size!");
                                return;
                            }else{
                                argTmp += rawString[j];
                            }
                        }else if(rawString[j] == ',' && bracketDepth==1){
                            argv.push_back(argTmp);
                            argTmp.clear();
                        }else{
                            argTmp += rawString[j];
                        }
                    }
                }else{
                    // in absence of operators, first bracket is expected to be a function
                    throw runtime_error("Badly formatted function: " + rawString+"\nExpected function name, got:"+rawString.substr(0,i));
                }                
            }
        }

        throw runtime_error("Failed to initialise function:"+rawString);
    }

    bool FuncNode::contains(shared_ptr<Path> path, string const& name) {
        size_t i=0;
        for(auto& uz:inputs){
            if(uz->contains(path, name)){
                path->addPoint(cast_node<FuncNode>(shared_from_this()), i); 
                return true;
            }
            i++;
        }
        return false;
    }

    
    void FuncNode::setVariableLink(string const& name, double *link) {
        for(auto& uz:inputs){
            uz->setVariableLink(name, link);
        }
    }
    
    bool FuncNode::contains(string const& name) {
        return func.contain(inputs, name);
    }

    string FuncNode::getFuncName(){
        return func.getFuncName();
    }

    SimpleFunc::FuncTypes FuncNode::getFuncType(){
        return func.getFuncType();
    }

    void FuncNode::getVariables(vector<string>& inp) {
        // recursion into function children
        for(auto& uz:inputs){
            uz->getVariables(inp);
        }
    }

    void FuncNode::setName(string const& name, string const& newName){
        for (auto& i:inputs){
            i->setName(name, newName);
        }
    }

    node_container<Node> FuncNode::differ(string const& varName) {
        return func.differ(cast_node<FuncNode>(shared_from_this()), varName);
    }

    int FuncNode::numOfContains(string const& varName) {
        int out=0;
        for(auto& uz:inputs){
            out += uz->numOfContains(varName);
        }
        return out;
    }

    void FuncNode::removeInp(int ind){
        clear_node(inputs[ind]);

        inputs.erase(inputs.begin() + ind);
        gain.erase(gain.begin() + ind);

        input.pop_back();
    }

    size_t FuncNode::getRank(){
        return getInputs().size();
    }

    void FuncNode::invertNode(node_container<Node> next, size_t refIndex){
        // invert gains
        for(size_t i=0; i<inputs.size(); i++){
            // ref gain remains as is
            if(i != refIndex){
                gain[i] = gain[i]*-1*gain[refIndex];
            }
        }

        clear_node(inputs[refIndex]);
        inputs[refIndex] = next;
        // TODO IT MAY BE A FUNCTION, THEN LIFE is NOT SO EASY!

//            if(this.func.flipFlag(refIndex)){
//                int tIndx;
//                if(refIndex==1){
//                    tIndx=0;
//                }else{
//                    tIndx=1;
//                }
//                Node temp=this.inputs.get(tIndx);
//                this.inputs.set(refIndex, temp);
//                this.inputs.set(tIndx,next);
//            }else{
//                this.inputs.set(refIndex, next);
//            }
//            this.func.inverse(refIndex);
    }

    node_container<Node> FuncNode::simplify(){
        auto output = func.simplify(cast_node<FuncNode>(shared_from_this()));
        return output;

//        //Check if all inps is const
//        boolean flag=true;
//        for(size_t i=0;i<output.getInputs().size();i++){
//            if(!(this.inputs.get(i) instanceof Const)){
//                flag=false;
//                break;
//            }
//        }
//        if(flag){
////                System.err.println(this.asString());
//            return new Const(this.getValue(0, null, null));
//        }else{
////                System.err.println(this.asString());
//            return this;
//        }

    }

    node_container<Node> FuncNode::expand(){
        auto output = func.expand(cast_node<FuncNode>(shared_from_this()));

        if(output->type() == FunctionNode){
            auto rp = cast_node<FuncNode>(output);
            output = rp->simplify();
        }
        
        return output;
    }

    void FuncNode::addOperand(node_container<Node> oper, int gain){
        inputs.push_back(oper);
        this->gain.push_back(gain);
        input.push_back(0);
    }

    double FuncNode::getValue() {
        for(size_t i=0; i<getInputs().size(); i++){
            input[i] = inputs[i]->getValue();
        }

        return func.Evaluate(gain, input);
    }

    void FuncNode::replaceVar(string const& name, node_container<Node> const& replacement) {
        for(size_t i=0; i<inputs.size(); i++){
            if(inputs[i]->type() == VariableNode){
                auto var = cast_node<Variable>(inputs[i]);
                if(var->getName() == name){
                    if (var == replacement){
                        // prevent deallocation
                        continue;
                    }

                    clear_node(inputs[i]);
                    inputs[i] = replacement->copy();
                }
            }else{
                inputs[i]->replaceVar(name, replacement);
            }
        }
        //simplify();
    }

    node_container<Node> FuncNode::copy() {
        return make_node<FuncNode>(*this);
    }

    string FuncNode::toString() {
        string output="";
        if (getRank() == 0)
            // no inputs (fo some reason); therefore nothing to print
            return output;

        if (isOperator(func.getFuncType())){
            // case of inp1 operator inp2 operator ...
            switch (func.getFuncType()){
                case SimpleFunc::tSum:
                    if (gain[0] == -1)
                        output = "-";
                    break;
                case SimpleFunc::tSubtract:
                    if (gain[0] == 1)
                        output = "-";
                    break;
                case SimpleFunc::tMult:
                    if (gain[0] == -1)
                        output = "1/";
                    break;
                case SimpleFunc::tDivide:
                    if (gain[0] == 1)
                        output = "1/";
                    break;
                default:
                    break;
            }
            for(size_t i=0; i<inputs.size(); i++){
                // first operator is already added
                if (i != 0)
                    output += func.getFuncName(gain[i]);

                if(inputs[i]->type() == FunctionNode){
                    output += "(";
                    output += inputs[i]->toString();
                    output += ")";
                }else{
                    // const or varname; add without brackets
                    output += inputs[i]->toString();
                }

            }
        }else{
            // simple case of fName(inp1, inp2, ...)
            vector<string> inps(inputs.size());
            for (auto& n:inputs)
                inps.push_back(n->toString());
            output = func.getFuncName() + "(" + XmlParser::join(inps,", ") + ")";
        }
        
        return output;
    }

    GraphNodeType FuncNode::type() {
        return GraphNodeType::FunctionNode;
    }

    /**
     * @return the inputs
     */
    const vector<node_container<Node>>& FuncNode::getInputs() {
        return inputs;
    }

    void FuncNode::setInput(node_container<Node> value, size_t idx){
        if(idx+1 > inputs.size())
            throw "FuncNode::setGain - out of bounds!";

        inputs[idx] = value;
    }

    /**
     * @return the gain
     */
    const vector<int>& FuncNode::getGain() {
        return gain;
    }

    void FuncNode::setGain(int value, size_t idx){
        if(idx+1 > gain.size())
            throw "FuncNode::setGain - out of bounds!";

        gain[idx] = value;
    }

    void FuncNode::debugInfo(int depth) {
        qDebug().noquote() << QString("    ").repeated(depth) << "Func: " << func.getFuncName() << " addr:" << (size_t)this;
        for (size_t i=0; i<inputs.size(); i++){
            qDebug().noquote() << QString("    ").repeated(depth+1) << "gain:" << gain[i];
            inputs[i]->debugInfo(depth+1);
        }
    }
}

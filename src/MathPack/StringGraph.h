#ifndef MATHPACK_SG_H
#define MATHPACK_SG_H

#include <vector>
#include <string>
#include <type_traits>
#include <concepts>
#include <unordered_set>
#include <stdexcept>
#include <memory>
#include <algorithm>
#include "WorkSpace.h"
#include "MathFunction.h"

using namespace std;

namespace MathPack{
    enum GraphNodeType {ConstNode, VariableNode, FunctionNode};

    template<typename T>
    concept ptr_to_node = std::is_pointer_v<T> && std::is_base_of_v<Node, std::remove_pointer_t<T>>;

    template<typename T>
    concept shared_ptr_to_node =
        requires {
            typename T::element_type; // Only exists in smart pointers like shared_ptr
        } &&
        std::is_base_of_v<Node, typename T::element_type> ||
        ptr_to_node<T>;

    // Variadic concept across parameter pack
    template<typename... Ts>
    concept node_vector = (shared_ptr_to_node<Ts> && ...);

    class FuncNode;

    class Path{
        vector<node_container<FuncNode>> uzelPath;
        vector<size_t> numberOfOperand;
        
        public:
            Path();

            const vector<node_container<FuncNode>>& getFuncs();

            size_t length();

            /**
             * Get i-th node in the path
             */
            node_container<FuncNode> getNode(size_t const);

            void addPoint(node_container<FuncNode> func, size_t numOfOper);

            string toString();

        friend class StringGraph;
    };

    /**
     * Classifies input (const, var or func) and cretes appropriate node object
     */
    node_container<Node> createNode(string str);

    class Node : public enable_shared_from_this<Node>{
        public:
            virtual ~Node() = default;
            virtual void setName(string const& name, string const& newName) = 0;
            virtual double getValue() = 0;
            virtual void replaceVar(string const& name, node_container<Node> const& replace) = 0;
            virtual bool contains(shared_ptr<Path> path, string const& varName) = 0;
            virtual bool contains(string const& varName) = 0;
            virtual int numOfContains(string const& varName) = 0;
            virtual void setVariableLink(string const& name, double *link) = 0;
            virtual node_container<Node> differ(string const& varName) = 0;

            /**
             * append list by variable this Node contains
             */
            virtual void getVariables(vector<string>& list) = 0;

            virtual node_container<Node> copy() = 0;
            virtual string toString() = 0;
            virtual GraphNodeType type() = 0;

            virtual void debugInfo(int depth=0) = 0;
    };

    class FuncNode : virtual public Node{
        private:
            MathFunction func;
            vector<node_container<Node>> inputs;
            vector<int> gain;

        public:
            /**
             * cache of function's input numerical values
             */
            vector<double> input;


        public:
            FuncNode(string rawString);
            ~FuncNode();

            //template<node_vector... T>
            template<typename... T>
            FuncNode(string const& function, const node_container<T>&... input);
            FuncNode(string const& function, vector<node_container<Node>> input, vector<int> gains=vector<int>());

            FuncNode(FuncNode & input);
            FuncNode& operator=(FuncNode & other);
            

            string getFuncName();
            SimpleFunc::FuncTypes getFuncType();
            size_t getRank();

            void invertNode(node_container<Node> next, size_t refIndex);
            /**
             * In case self will be rebuilt, it will be updated via reverence
             */
            node_container<Node> simplify();
            /**
             * In case self will be rebuilt, it will be updated via reverence
             */
            node_container<Node> expand();

            /**
             * @return the inputs
             */
            const vector<node_container<Node>>& getInputs();
            void addOperand(node_container<Node> oper, int gain);
            void setInput(node_container<Node> value, size_t idx);
            void removeInp(int ind);

            /**
             * @return the gain
             */
            const vector<int>& getGain();
            void setGain(int value, size_t idx);

            virtual void setName(string const& name, string const& newName) override;
            virtual double getValue() override;
            virtual void replaceVar(string const& name, node_container<Node> const& replace) override;
            virtual bool contains(shared_ptr<Path> path, string const& varName) override;
            virtual bool contains(string const& varName) override;
            virtual int numOfContains(string const& varName) override;
            virtual void setVariableLink(string const& name, double *link) override;
            virtual node_container<Node> differ(string const& varName) override;
            virtual void getVariables(vector<string>& list) override;
            virtual node_container<Node> copy() override;
            virtual string toString() override;
            virtual GraphNodeType type() override;

            virtual void debugInfo(int depth=0) override;
    };

    class Const : virtual public Node{
        private:
            double value;

        public:
            Const(double value);
            Const(string const& str);

            void setValue(double newVal);

            virtual void setName(string const& name, string const& newName) override;
            virtual double getValue() override;
            virtual void replaceVar(string const& name, node_container<Node> const& replace) override;
            virtual bool contains(shared_ptr<Path> path, string const& varName) override;
            virtual bool contains(string const& varName) override;
            virtual int numOfContains(string const& varName) override;
            virtual void setVariableLink(string const& name, double *link) override;
            virtual node_container<Node> differ(string const& varName) override;
            virtual void getVariables(vector<string>& list) override;
            virtual node_container<Node> copy() override;
            virtual string toString() override;
            virtual GraphNodeType type() override;

            virtual void debugInfo(int depth=0) override;
    };

    class Variable : virtual public Node{
        private:
            string name;
            int index=-1,
                secondIndex;
            double* value=nullptr; // from ws link, for quicker access
            WorkSpace::Variable *workSpaceLink = nullptr;

        public:
            Variable(string const& name);
            Variable(string const& name, WorkSpace::Variable *dataLink);

            const string& getName();
            void setName(string const& newName);

            WorkSpace::Variable* getWsLink(); 
            void setWsLink(WorkSpace::Variable* wsVar);

            virtual void setName(string const& name, string const& newName) override;
            virtual double getValue() override;
            virtual void replaceVar(string const& name, node_container<Node> const& replace) override;
            virtual bool contains(shared_ptr<Path> path, string const& varName) override;
            virtual bool contains(string const& varName) override;
            virtual int numOfContains(string const& varName) override;
            virtual void setVariableLink(string const& name, double *link) override;
            virtual node_container<Node> differ(string const& varName) override;
            virtual void getVariables(vector<string>& list) override;
            virtual node_container<Node> copy() override;
            virtual string toString() override;
            virtual GraphNodeType type() override;

            virtual void debugInfo(int depth=0) override;
    };

    class StringGraph {
        /**
         * Variables: i.N - electric currens; p.N - electric potential; X.N - state;
         * d.X.N - state differ; O.N - output; I.N - input.
         * @author Ivan
         */
        private:
            node_container<Node> root = nullptr;
            vector<string> variables; // cache


        private:
            shared_ptr<Path> findPath(string const& varName);

            /**
             * Note: doesn't make a copy of graph!
             */
            void multiply(shared_ptr<StringGraph> graph);

        public:
            static shared_ptr<StringGraph> mul(shared_ptr<StringGraph> gr, double value);
            static shared_ptr<StringGraph> mul(shared_ptr<StringGraph> gr1, shared_ptr<StringGraph> gr2, int gain);

            static shared_ptr<StringGraph> sum(vector<shared_ptr<StringGraph>> const& gr, vector<int> gains=vector<int>());
            static shared_ptr<StringGraph> sum(shared_ptr<StringGraph> gr1, shared_ptr<StringGraph> gr2, int gain);

            /**
             * Devides value/gr
             * @param value
             * @param gr
             * @return out=value/gr
             */
            static shared_ptr<StringGraph> divide(int value, shared_ptr<StringGraph> gr);

            StringGraph(string const& rawString);
            StringGraph(double val);
            StringGraph(WorkSpace::Variable *var, string const& customVarName=string());
            StringGraph(node_container<Node> uz);

            StringGraph(StringGraph const& input);
            StringGraph& operator=(StringGraph const& other);

            ~StringGraph();


            shared_ptr<StringGraph> getDiffer(string const& varName);

            shared_ptr<StringGraph> getFullTimeDiffer(vector<string> const& glabalVarNames,
                                                    //vector<shared_ptr<WorkSpace::Variable>> const& varList,
                                                    vector<shared_ptr<WorkSpace::DifferentialVar>> const& diffList);

            void linkVariableToWorkSpace(string const& name, WorkSpace::Variable *link);

            void simplify();

            void expand();

            string toString();

            /**
             * Check if this graph contains only X.i, d.X.i, time.
             * @return
             */
            bool isInvariant();

            int canGet(string const& varType, int numOfVars);

            bool canGet(string const& varName);

            /**
             * Note: takes ownership of graph's root
             */
            void add(shared_ptr<StringGraph> graph);

            /**
             * Returns this-graph
             * Note: takes ownership of graph's root
             * @param graph
             */
            void sub(shared_ptr<StringGraph> graph);

            /**
             * this = this-inp
             * Note: takes ownership of Node
             * unused; remove?
             * @param inp
             */
            // void sub(node_container<Node> inp);

            /**
             * unused
             */
            //void divide(shared_ptr<StringGraph> graph);

            void multiply(double value);

            void replaceVariable(string const& name, shared_ptr<StringGraph> replacement);

            void replaceWithConst(string const& name,double value);

            /**
             * Renames all occurances of VariableNodes in the graph
             */
            void renameVariable(string const& name, string const& newName);

            bool contains(string const& var);

            int numOfContains(string const& var);

            /**
             * Convert this graph to become: name = graph(args...)
             * Case: k*name = f(name,u)
             * @param name
             * @param k
             */
            void getVariable(string const& name, int k);

            /**
             * Convert this graph to become: name = graph(args...)
             * Case: left = f(name, u,...)
             * @param name
             * @param left
             */
            void getVariable(string const& name, shared_ptr<StringGraph> left);

            /**
             * Convert this graph to become: name = graph(args...)
             * case 0 = f(name, u,...) ???
             */
            void getVariable(string const& name);

            double evaluate() const;

            vector<string> getVariableNames();
            vector<node_container<Variable>> getVariables();

            node_container<Node> getRoot();
    };
}

#include "StringGraph_templates.h"

#endif

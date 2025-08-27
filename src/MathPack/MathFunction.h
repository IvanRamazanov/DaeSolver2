#ifndef MATHPACK_MAHTFUNC_H
#define MATHPACK_MAHTFUNC_H

#include <string>
#include <vector>
#include <memory>


/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

using namespace std;

namespace MathPack{
    // Node memory storage
    template <typename T>
    using node_container = std::shared_ptr<T>;
    //using node_container = T*;

    template <typename T, typename... Args>
    node_container<T> make_node(Args&&... args) {
        if constexpr (std::is_same_v<node_container<T>, std::shared_ptr<T>>) {
            return std::make_shared<T>(std::forward<Args>(args)...);
        } else {
            return new T(std::forward<Args>(args)...);
        }
    }

    template <typename T, typename V>
    node_container<T> cast_node(node_container<V> basePtr) {
        if constexpr (std::is_same_v<node_container<V>, std::shared_ptr<V>>) {
            return dynamic_pointer_cast<T>(basePtr);
        } else {
            return dynamic_cast<T*>(basePtr);
        }
    }

    template <typename T>
    void clear_node(node_container<T> basePtr) {
        if constexpr (std::is_pointer_v<decltype(basePtr)>) {
            delete basePtr;
        }
    }


    class Node; // StringGraph.h
    class FuncNode;

    class SimpleFunc{
        // TODO can be templaterized (based on enum FuncTypes?)
        public:
            enum FuncTypes {tSum, tSubtract, tMult, tDivide, tSine, tASin, tExp, tLogn, tCos, tGZ, tIF, tInverse, tPow};

            /**
             * Calculate f(input, gains)
             */
            virtual double Evaluate(vector<int> const& gains, vector<double> const& input) = 0;

            /**
             * Return inverse function rely on operand index
             * @param index
             * @return
             */
            virtual shared_ptr<SimpleFunc> _inverse(int index) = 0;

            /**
             * @param i gain: 1 - this function; -1 - inverse function.
             */
            virtual string getName(int i) = 0;

            /**
             * May return same data (untouched)
             */
            virtual node_container<Node> simplify(node_container<FuncNode> uz) = 0;

            /**
             * May return same data (untouched)
             */
            virtual node_container<Node> expand(node_container<FuncNode> uz) = 0;

            /**
             * Will always return new data (at least a copy)
             */
            virtual node_container<Node> differ(node_container<FuncNode> root, string const& varName) = 0;

            virtual vector<int> getRequiredIndexes() = 0;

            virtual FuncTypes type() = 0;
    };

    class Subtraction : virtual public SimpleFunc {
        public:
            double Evaluate(vector<int> const& gains, vector<double> const& input) override;
            shared_ptr<SimpleFunc> _inverse(int index) override;
            node_container<Node> simplify(node_container<FuncNode> input) override;
            node_container<Node> expand(node_container<FuncNode> uz) override;
            string getName(int i) override;
            vector<int> getRequiredIndexes() override;
            node_container<Node> differ(node_container<FuncNode> root, string const& varName) override;
            FuncTypes type() override;
    };

    class Summa : virtual public SimpleFunc{
        public:
            double Evaluate(vector<int> const& gains, vector<double> const& input) override;
            shared_ptr<SimpleFunc> _inverse(int index) override;
            node_container<Node> simplify(node_container<FuncNode> input) override;
            node_container<Node> expand(node_container<FuncNode> input) override;
            string getName(int i) override;
            vector<int> getRequiredIndexes() override;
            node_container<Node> differ(node_container<FuncNode> root, string const& varName) override;
            FuncTypes type() override;
    };

    class Multiply : virtual public SimpleFunc{
        public:
            double Evaluate(vector<int> const& gains, vector<double> const& input) override;
            shared_ptr<SimpleFunc> _inverse(int index) override;
            string getName(int i) override;
            node_container<Node> simplify(node_container<FuncNode> input) override;
            node_container<Node> expand(node_container<FuncNode> input) override;
            vector<int> getRequiredIndexes() override;

            /**
             * d.a*b+a*d.b
             * @param root Node that call differ method
             * @param varName
             */
            node_container<Node> differ(node_container<FuncNode> root, string const& varName) override;
            FuncTypes type() override;
    };

    class Divide : virtual public SimpleFunc{
        public: 
            double Evaluate(vector<int> const& gains, vector<double> const& input) override;
            shared_ptr<SimpleFunc> _inverse(int index) override;
            node_container<Node> simplify(node_container<FuncNode> input) override;
            node_container<Node> expand(node_container<FuncNode> uz) override;
            string getName(int i) override;
            vector<int> getRequiredIndexes() override;
            node_container<Node> differ(node_container<FuncNode> root, string const& varName) override;
            FuncTypes type() override;
    };

    class Sin : virtual public SimpleFunc{
        public:
            double Evaluate(vector<int> const& gains, vector<double> const& input) override;
            shared_ptr<SimpleFunc> _inverse(int index) override;
            string getName(int i) override;
            node_container<Node> simplify(node_container<FuncNode> input) override;
            node_container<Node> expand(node_container<FuncNode> uz) override;
            vector<int> getRequiredIndexes() override;
            node_container<Node> differ(node_container<FuncNode> root, string const& varName) override;
            FuncTypes type() override;
    };

    class ArcSin : virtual public SimpleFunc{
        public:
            double Evaluate(vector<int> const& gains, vector<double> const& input) override;
            shared_ptr<SimpleFunc> _inverse(int index) override;
            string getName(int i) override;
            node_container<Node> simplify(node_container<FuncNode> input) override;
            node_container<Node> expand(node_container<FuncNode> uz) override;
            vector<int> getRequiredIndexes() override;
            node_container<Node> differ(node_container<FuncNode> root, string const& varName) override;
            FuncTypes type() override;
    };

    class Exp : virtual public SimpleFunc{
        public:
            double Evaluate(vector<int> const& gains, vector<double> const& input) override;
            shared_ptr<SimpleFunc> _inverse(int index) override;
            string getName(int i) override;
            node_container<Node> simplify(node_container<FuncNode> input) override;
            node_container<Node> expand(node_container<FuncNode> uz) override;
            vector<int> getRequiredIndexes() override;
            node_container<Node> differ(node_container<FuncNode> root, string const& varName) override;
            FuncTypes type() override;
    };

    class Logn : virtual public SimpleFunc{
        public:
            double Evaluate(vector<int> const& gains, vector<double> const& input) override;
            shared_ptr<SimpleFunc> _inverse(int index) override;
            string getName(int i) override;
            node_container<Node> simplify(node_container<FuncNode> input) override;
            node_container<Node> expand(node_container<FuncNode> uz) override;
            vector<int> getRequiredIndexes() override;

            /**
             * (ln(x))'=(1/x)*(x')
             * a=1/x, b=x'.
             * @param root
             * @param varName
             * @return
             */
            node_container<Node> differ(node_container<FuncNode> root, string const& varName) override;
            FuncTypes type() override;
    };

    class GreatThan : virtual public SimpleFunc{
        public:
            double Evaluate(vector<int> const& gains, vector<double> const& input) override;
            shared_ptr<SimpleFunc> _inverse(int index) override;
            string getName(int i) override;
            node_container<Node> simplify(node_container<FuncNode> input) override;
            node_container<Node> expand(node_container<FuncNode> uz) override;
            vector<int> getRequiredIndexes() override;
            node_container<Node> differ(node_container<FuncNode> root, string const& varName) override;
            FuncTypes type() override;
    };

    class If : virtual public SimpleFunc{
        public:
            double Evaluate(vector<int> const& gains, vector<double> const& input) override;
            shared_ptr<SimpleFunc> _inverse(int index) override;
            string getName(int i) override;
            node_container<Node> simplify(node_container<FuncNode> input) override;
            node_container<Node> expand(node_container<FuncNode> uz) override;
            vector<int> getRequiredIndexes() override;
            node_container<Node> differ(node_container<FuncNode> root, string const& varName) override;
            FuncTypes type() override;
    };

    class Inverse : virtual public SimpleFunc{
        public:
            double Evaluate(vector<int> const& gains, vector<double> const& input) override;
            shared_ptr<SimpleFunc> _inverse(int index) override;
            string getName(int i) override;
            node_container<Node> simplify(node_container<FuncNode> input) override;
            node_container<Node> expand(node_container<FuncNode> uz) override;
            vector<int> getRequiredIndexes() override;
            node_container<Node> differ(node_container<FuncNode> root, string const& varName) override;
            FuncTypes type() override;
    };

    class Pow : virtual public SimpleFunc{
        public:
            double Evaluate(vector<int> const& gains, vector<double> const& input) override;
            shared_ptr<SimpleFunc> _inverse(int index) override;
            string getName(int i) override;
            node_container<Node> simplify(node_container<FuncNode> input) override;
            node_container<Node> expand(node_container<FuncNode> uz) override;
            vector<int> getRequiredIndexes() override;
            node_container<Node> differ(node_container<FuncNode> root, string const& varName) override;
            FuncTypes type() override;
    };

    class Cos : virtual public SimpleFunc {
        public:
            double Evaluate(vector<int> const& gains, vector<double> const& input) override;
            shared_ptr<SimpleFunc> _inverse(int index) override;
            string getName(int i) override;
            node_container<Node> simplify(node_container<FuncNode> input) override;
            node_container<Node> expand(node_container<FuncNode> uz) override;
            vector<int> getRequiredIndexes() override;
            node_container<Node> differ(node_container<FuncNode> root, string const& varName) override;
            FuncTypes type() override;
    };

    class MathFunction {
        private:
            int rank;
            shared_ptr<SimpleFunc> function=nullptr;
            string name;

        public:
            MathFunction(); // Note, only for FuncNode constructor

            MathFunction(string const& functionName);

            // MathFunction(MathFunction const& input);
            // MathFunction(MathFunction && input);
            // MathFunction& operator=(MathFunction const& input);
            // MathFunction& operator=(MathFunction && input);

            // ~MathFunction();

            static bool isSimple(MathFunction *inp);

            void inverse(int index);

            const string& getFuncName();

            // Only for toString
            string getFuncName(int i);

            SimpleFunc::FuncTypes getFuncType();

            node_container<Node> simplify(node_container<FuncNode> inp);

            node_container<Node> expand(node_container<FuncNode> inp);

            double Evaluate(vector<int> gains, vector<double> input);

            int getRank();

            node_container<Node> differ(node_container<FuncNode> root, string const& varName);

            bool contain(vector<node_container<Node>> inp, string const& varName);

    };
}

#include "StringGraph.h"

#endif

#ifndef MATHPACK_WS_H
#define MATHPACK_WS_H

#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include "Parser.h"
#include "../ElementBase/Parameters/Parameter.h" // for InitParam

using namespace std;

/**
 *
 * @author Ivan Ramazanov
 */
namespace WorkSpace{
    enum WsTypes {Scalar, Vector, Matrix, Differential};
    enum WsDataType {Flow, Potential, Discrete};
    constexpr auto TIME_VAR_NAME = "t";
    constexpr auto DIFF_PTRFIX = "d.";

    class Variable{
        protected:
            string name;
            vector<double> value={0}; // TODO switch to raw pointers, to enable pin to WS linkage (WS value bypass)

            size_t size = 1;
            vector<double> initValue={0};

        public:
            const bool isConst=false;
            const WsDataType dataType = Discrete;

        protected:
            Variable();

        public:
            Variable(string const& name, double value, bool isConst=false, WsDataType type=Discrete);
            Variable(string const& name, vector<double> const& initVal);

            void setValue(double val);
            void setValue(vector<double> const& val);

            /**
             * sets/resets inital value
             */
            virtual void init();
            void init(double value);
            void init(vector<double> const& val);

            double getValue();
            double* getRef();
            size_t getSize();

            const string& getName();

            virtual WsTypes type();
    };

    class DifferentialVar : public Variable {
        private:
            ElementBase::InitParam *initVal=nullptr;
        public:
            const shared_ptr<Variable> baseVar;

        public:
            DifferentialVar(shared_ptr<Variable> baseVar, WsDataType type=Discrete);

            WsTypes type() override;

            void setInitVal(ElementBase::InitParam *initVal);
            ElementBase::InitParam* getInitVal();
    };

    class WorkSpace {
        private:
            vector<shared_ptr<Variable>> variableList;

        public:
            WorkSpace();

            double get(int indx);

            static bool isDynamicVariable(Variable *var);

            vector<string> getVarNameList();

            string getName(int i);

            void add(shared_ptr<Variable> var);
            Variable* add(string const& name, double value, bool isconst=false, WsDataType type=Discrete);

            /**
             * Creates diff for baseName var. If baseName doesn't exist in WS, it will be created.
             */
            DifferentialVar* addDiff(string const& baseName, WsDataType type);

            Variable* addVector(string const& name, vector<double> const& value);

            const vector<shared_ptr<Variable>>& getVarList() const;

            Variable* get(string const& name) const;

        private:
            bool contains(string const& name) const;
    };
}

#endif

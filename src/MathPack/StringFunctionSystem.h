#ifndef MATHPACK_SFS_H
#define MATHPACK_SFS_H

#include <string>
#include <vector>
#include <memory>
#include "StringGraph.h"
#include "../ElementBase/ElemBaseDecl.h"


using namespace std;

namespace MathPack{
    /**
     *
     * @author Ivan
     */
    class StringFunctionSystem {
        public:
            vector<shared_ptr<StringGraph>> eqSystem; // 'normilized' to 0 = function equations
            vector<shared_ptr<WorkSpace::Variable>> flows, potns;
            vector<shared_ptr<WorkSpace::DifferentialVar>> diffVars;


        public:
            /**
             * @param pinCounter - counter to keep track of position in workspace vector (flows & potentials)
             * @param ws - to check for unknown variables (global WS that has, for e.g., simulation time variable)
             */
            StringFunctionSystem(ElementBase::Element *element, WorkSpace::WorkSpace *ws);

            StringFunctionSystem(vector<StringFunctionSystem> funcList);
            
            void renameVariable(string const& oldName, string const& newName);

            void linkVarToWS(string const& name, WorkSpace::Variable *link);

        private:
            /**
             * String to int. if brackets are present, "[]" converts string before brackets
             */
            static int parseInt(string str);
    };
}

#include "../ElementBase/Element.h"

#endif

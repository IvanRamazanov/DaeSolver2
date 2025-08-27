#include "SolversFactory.h"

#include "Euler.h"
#include "Adams4.h"
#include "BDF1.h"
#include "BDF2.h"
#include "RK4.h"
#include "Rosenbrok.h"
#include "RungeKuttaFehlberg.h"
#include "TR.h"
#include "TRBDF.h"

using Factory = std::function<unique_ptr<Solvers::Solver>()>;

namespace Solvers{
    unique_ptr<Solver> getSolver(string name){
        static const std::unordered_map<std::string, Factory> factories = {
            {"Adams4",   [] { return make_unique<Adams4>(); }},
            {"BDF1",     [] { return make_unique<BDF1>(); }},
            {"BDF2",   [] { return make_unique<BDF2>(); }},
            {"Euler",  [] { return make_unique<Euler>(); }},
            {"RK4",    [] { return make_unique<RK4>(); }},
            {"Rosenbrok",    [] { return make_unique<Rosenbrok>(); }},
            {"RungeKuttaFehlberg",    [] { return make_unique<RungeKuttaFehlberg>(); }},
            {"TR", [] { return make_unique<TR>(); }},
            {"TRBDF",  [] { return make_unique<TRBDF>(); }}
        };

        if (auto it = factories.find(name); it != factories.end()) {
            return it->second();
        }else{
            throw runtime_error("Unknown solver: "+name);
        }
    }
}

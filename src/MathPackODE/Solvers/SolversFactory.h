#ifndef MATHODE_SOLVER_FACTORY_H
#define MATHODE_SOLVER_FACTORY_H

#include <memory>
#include <string>

namespace Solvers{
    class Solver;

    std::unique_ptr<Solver> getSolver(std::string name);
}

#endif

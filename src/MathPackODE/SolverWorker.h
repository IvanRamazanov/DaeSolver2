/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#ifndef ODE_SOLVER_WORKER_H
#define ODE_SOLVER_WORKER_H

#include <QObject>
#include <chrono>
#include "../UI/ModelState.h"
#include "../UI/Logger.h"

namespace Solvers{
    class Solver;
}

/**
 *
 * @author Ivan
 *
 */

using namespace std;

namespace dae_solver {
    class SolverWorker : public QObject {
        Q_OBJECT

        private:
            unique_ptr<Solvers::Solver> solver;
            ModelState *state;
            shared_ptr<Logger> logger;

        public:
            const bool IS_LOGGING = false;

            SolverWorker(ModelState *state, shared_ptr<Logger> logger);

        signals:
            void success();
            void progressIncrement(int const& value);
            void failed();
            void cancelled();
            void finished();
        
        public slots:
            void call();
            void abort();
    };
}

#include "./Solvers/Solver.h"

#endif

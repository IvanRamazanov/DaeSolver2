/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "SolverWorker.h"

#include "./Solvers/SolversFactory.h"
#include "Compiler.h"

/**
 *
 * @author Ivan
 *
 */

using namespace std;

namespace dae_solver {

    SolverWorker::SolverWorker(ModelState *state, shared_ptr<Logger> logger): state(state), logger(logger) {
        solver = Solvers::getSolver(state->getSolver());
    }

    void SolverWorker::call(){
        try{
            //double tEnd = state->getTend();
            //updateMessage("Compiling...");
            auto sys = Compiler(state->getMainSystem(), logger);
            emit progressIncrement(0);
            solver->init(sys, state, this, logger);
            auto start = chrono::system_clock::now();
            // Solver.progress.addListener((t, o, n)->{
            //     emit progressIncrement(n/tEnd*100.0);
            // });

            solver->solve();

            if(IS_LOGGING){
                //sys.layout();
            }
            
            auto elapsed = chrono::system_clock::now() - start;
            cout << "eval time: " << (elapsed.count() / 1e9) << "sec" << endl;
            emit success();
        }catch(exception &e){
            cerr << e.what();
            emit failed();
        }
        emit finished();
    }

    void SolverWorker::abort(){
        emit cancelled();
    }
}

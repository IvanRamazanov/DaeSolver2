#ifndef MATHODE_SOLVER_H
#define MATHODE_SOLVER_H

#include <vector>
#include <cmath>
#include "../../MathPack/Workspace.h"
#include "../DAE.h"
#include "../DiscreteSys.h"
#include "../SolverWorker.h"
#include "../../UI/Logger.h"
#include "../Compiler.h"
#include "../../MathPack/MatrixEqu.h"

using namespace std;

namespace Solvers{

    class Solver {
        private:
            //vector<Connections::WireCluster*> wires;
            vector<vector<double>> J;
            vector<double> testFac;
            bool testFlag;
            shared_ptr<dae_solver::Logger> logger;
            vector<ElementBase::Element*> elems;

        protected:
            vector<shared_ptr<MathPack::StringGraph>> newtonFunc, algSystem;
            vector<vector<shared_ptr<MathPack::StringGraph>>> symbJacobian,invJacob;
            shared_ptr<WorkSpace::WorkSpace> vars;
            shared_ptr<dae_solver::DAE> dae;
            shared_ptr<dae_solver::DiscreteSystem> discSys;
            vector<double*> Xvector, dXvector, stateVector;
            int jacobEstType, diffRank;
            vector<double> vals;
            dae_solver::SolverWorker *workerThread;
            bool cancelFlag = false;
        public:
            double dt = 0.01,
                    tEnd = 10,
                    absTol = 1e-8,
                    relTol = 1e-3;
            WorkSpace::Variable *time;
            double progress = 0;
            
        
        private:
            void preEvaluate();

            void postEvaluate();

            void updateOutputs();

        protected:
            void evalMathDX();

            virtual void selfInit();

            void copyArray(vector<double*> const& source, vector<double> & destination);

            /**
             * Evaluates estimation of Jacobian of fx near f0=fx(x0) point.
             * @param J - output
             * @param fx - target function
             * @param f0 - function value at fx(x0)
             * @param x - x0 initial point
             * @param fac
             */
            virtual void estimJ(vector<vector<double>> & J,
                        vector<shared_ptr<MathPack::StringGraph>> const& fx,
                        vector<double> const& f0,
                        vector<double*> & x0,
                        vector<double> & fac);

            void updateJ(vector<vector<double>> & J, vector<double> const& fNew, vector<double> const& fOld, vector<double> const& xDelta);

        public:
            Solver();

            virtual void evalNextStep() = 0;

            void init(dae_solver::Compiler & cpl,
                    dae_solver::ModelState *state,
                    dae_solver::SolverWorker *tsk,
                    shared_ptr<dae_solver::Logger> logger);

            void solve();

            void evalSysState();
    };
}

#endif

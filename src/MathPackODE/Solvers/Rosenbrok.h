#ifndef SOLVERS_ROSENBROK_H
#define SOLVERS_ROSENBROK_H

#include "Solver.h"

namespace Solvers{

    class Rosenbrok : public Solver {
        vector<double> dfdt,x0,F0,F1,F2,k1,k2,k3,xnew;
        vector<vector<double>> W,dfdx;
        vector<double> fac;
        double const d = 1.0/(2.0+sqrt(2.0)),
                        e32 = 6+sqrt(2.0),
                        eps = 2.220446049250313E-16,
                        sqrtEps = sqrt(eps),
    //            hmin=16.0*eps*tEnd,
                hmin = eps,
                hmax = 1e-3,
                mag = 1.0/3.0;

        int nSteps,nFailed;

        /**
         * x0 must be set
         */
        void estimT();

        double error();

        void evalW();

        protected:
            void selfInit() override;

            /**
             * Evaluates estimation of Jacobian of fx near f0=fx(x0) point.
             * @param J - output
             * @param fx - target function
             * @param f0 - function value at fx(x0)
             * @param vars - x0 initial point
             * @param fac
             */
            void estimJ(vector<vector<double>> & J,
                        vector<shared_ptr<MathPack::StringGraph>> const& fx,
                        vector<double> const& f0,
                        vector<double*> & x,
                        vector<double> & fac) override;

        public:
            Rosenbrok();

        void evalNextStep() override;
    };
}
#endif

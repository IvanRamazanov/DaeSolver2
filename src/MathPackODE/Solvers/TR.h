#ifndef SOLVERS_TR_H
#define SOLVERS_TR_H

#include "Solver.h"

namespace Solvers{

    class TR : public Solver {
        vector<double> x0,newtonF,df0,df1;
        vector<vector<double>> dfdx;
        double tolerance=1e-6;

        void makeTRJ(vector<vector<double>> & J);

        /**
         * fng=dXvector
         * xng=Xvector
         */
        void trF();

        protected:
            /**
             * Estimates d(d.X)/dX
             * @param J - output
             * @param fx - target function
             * @param f0 - function value at fx(x0)
             * @param x - x0 initial point
             * @param fac
             */
            void estimJ(vector<vector<double>> & J,
                        vector<shared_ptr<MathPack::StringGraph>> const& fx,
                        vector<double> const& f0,
                        vector<double*> & x,
                        vector<double> & fac) override;

            void selfInit() override;

        public:
            void evalNextStep() override;
    };
}
#endif

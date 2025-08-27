#ifndef SOLVERS_TRBDF_H
#define SOLVERS_TRBDF_H

#include "Solver.h"

namespace Solvers{

    class TRBDF : public Solver{
        vector<double> xn,
                newtonF,
                fn, // dx/dt=f(xn)=fn
                fng, // dx/dt=f(xng)=fng
                fnn,
                xng;
        vector<vector<double>> dF;
        double tolerance=1e-6,
                gamma=2.0-sqrt(2.0),
                hmax=1e-3,
                hmin=1e-16,
        //            gamma=1,
                kGamma=(-3.0*gamma*gamma+4.0*gamma-2.0)/(12.0*(2.0-gamma));

        void makeTRJ(vector<vector<double>> & J);
        void makeBDFJ(vector<vector<double>> & J);

        /**
         * fng=dXvector
         * xng=Xvector
         */
        void trF();

        /**
         * x_(n+1)=Xvector
         */
        void bdfF();

        protected:
            void selfInit() override;

            

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

        
        public:
            void evalNextStep() override;
        
    };
}
#endif

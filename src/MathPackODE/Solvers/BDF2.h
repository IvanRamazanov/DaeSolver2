#ifndef SOLVERS_BDF2_H
#define SOLVERS_BDF2_H

#include "Solver.h"

namespace Solvers{

    class BDF2 : public Solver {
        vector<double> xTemp, F, xOld;
        vector<vector<double>> dF;
        double tolerance=1e-7;

        void newtonF();

        protected:
            void selfInit() override;

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

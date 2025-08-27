#ifndef SOLVERS_BDF1_H
#define SOLVERS_BDF1_H

#include "Solver.h"

namespace Solvers{

    class BDF1 : public Solver {
        vector<double> temp,F;
        vector<vector<double>> dF;
        double tolerance = 1e-6;

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

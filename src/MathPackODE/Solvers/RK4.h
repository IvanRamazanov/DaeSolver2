#ifndef SOLVERS_RK4_H
#define SOLVERS_RK4_H

#include "Solver.h"

namespace Solvers{

    class RK4 : public Solver {
        double bTime;
        vector<double> bXVector,
                C={0, 1.0/2.0, 1.0/2.0, 1.0},
                B={1.0/6.0, 1.0/3.0, 1.0/3.0, 1.0/6.0};
        const int ORDER=4;
        const double A[3][3]={
                {1.0/2.0, 0, 0},
                {0, 1.0/2.0, 0},
                {0, 0, 1}
        };
        vector<vector<double>> K;

        void add(int step);

        protected:
            void selfInit() override;

        public:
            RK4();

            void evalNextStep() override;
        
    };
}
#endif






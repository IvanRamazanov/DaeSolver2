#ifndef SOLVERS_RKF_H
#define SOLVERS_RKF_H

#include "Solver.h"

namespace Solvers{

    class RungeKuttaFehlberg : public Solver {
        double bTime;
        vector<double> bXVector,
                        error,
                        relErr,
                        bCommVect;
        const double C[6]={0, 1.0/4.0, 3.0/8.0, 12.0/13.0, 1.0, 1.0/2.0},
                    B[6]={16.0/135.0, 0, 6656.0/12825.0, 28561.0/56430.0, -9.0/50.0, 2.0/55.0},
                    Br[6]={25.0/216.0, 0, 1408.0/2565.0, 2197.0/4104.0, -1.0/5.0, 0};
        const int ORDER=6;
        const double A[5][5]={
                {1.0/4.0, 0, 0, 0, 0},
                {3.0/32.0, 9.0/32.0, 0, 0, 0},
                {1932.0/2197.0, -7200.0/2197.0, 7296.0/2197.0, 0,0},
                {439.0/216.0, -8, 3680.0/513.0, -845.0/4104.0, 0},
                {-8.0/27.0, 2, -3544.0/2565.0, 1859.0/4104.0, -11/40}
        };
        vector<vector<double>> K;

        void add(int step);

        protected:
            void selfInit() override;

        public:
            RungeKuttaFehlberg();

            void evalNextStep() override;
    };
}
#endif

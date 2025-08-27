/*
 * The MIT License
 *
 * Copyright 2017 Ivan.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef MPACK_ODE_DAE_H
#define MPACK_ODE_DAE_H

#include <vector>
#include <memory>
#include <unordered_map>
#include <set>
#include <QFile>
#include <QString>
#include "../MathPack/StringGraph.h"
#include "../MathPack/WorkSpace.h"
#include "../MathPack/Parser.h"
#include "../MathPack/Vectors.h"
#include "../MathPack/StringFunctionSystem.h"
#include "../UI/Logger.h"

namespace dae_solver {

    /**
     *
     * @author Ivan
     */
    class DAE {
        // solver functions
        vector<shared_ptr<MathPack::StringGraph>> newtonFunc,
                                        algSystem;
        vector<vector<shared_ptr<MathPack::StringGraph>>> Jacob, invJacob;
        vector<std::string> stateVectorNames; // glabal names; for Jacobian

        // data mangment
        vector<shared_ptr<WorkSpace::Variable>> dynamicVars;
        vector<shared_ptr<WorkSpace::DifferentialVar>> dynDiffVars;
        vector<shared_ptr<MathPack::StringGraph>> stateToDynVars; // to convert from stateVector back to original
        unordered_map<WorkSpace::DifferentialVar*, shared_ptr<MathPack::StringGraph>> stateToDynDiffs; // could be nullptr!
        
        vector<double> logtime;
        vector<vector<double>> logdata;
        shared_ptr<Logger> logger;

        public:
            shared_ptr<WorkSpace::WorkSpace> workspace;
            size_t diffRank = 0,
                stateSize = 0;
            vector<double*> xVector,
                            dXVector,
                            stateVector;
            

        private:
            /**
             * @param varList - list of original names; to get global var index
             */
            vector<vector<int>> getConnectionMap(vector<ElementBase::Element*> Elems,
                                                std::string const& domainId,
                                                vector<std::string> const& globalNames,
                                                bool flowMap);

            void setVarsByKirhgof(vector<vector<int>> & connMap, vector<std::string> const& globalNames);

            /**
             * Generate var replacement vector: stateVar[i] = f(refVars...)
             * @param connMap Kirhgoff connection map
             * @param refVars list of independent vars within connection map
             * @return indexes of ODE vector vars (indexes of X, dX in state space)
             */
            set<size_t> replaceVars(vector<vector<int>> & connMap,
                            vector<std::string> const& globalNames,
                            set<size_t> & stateVecIdxs);

        public:
            DAE(vector<ElementBase::Element*> physElements,
                shared_ptr<WorkSpace::WorkSpace> ws,
                shared_ptr<Logger> logger);

            /**
             * @return the algSystem
             */
            vector<shared_ptr<MathPack::StringGraph>> getAlgSystem();

            void initJacobian(int jacobType);

            void layout();

            /**
             * @return the Jacob
             */
            vector<vector<shared_ptr<MathPack::StringGraph>>> getJacob();

            /**
             * @return the invJacob
             */
            vector<vector<shared_ptr<MathPack::StringGraph>>> getInvJacob();

            /**
             * @return the newtonFunc
             */
            vector<shared_ptr<MathPack::StringGraph>> getNewtonFunc();

            /**
             * refreshes values in element's WSes after model step
             */
            void updateElemWorkspaces();
    };
}

#endif

#ifndef MATHPACK_MTXEQ_H
#define MATHPACK_MTXEQ_H

#include <vector>
#include <stdexcept>
#include <cmath>
#include <utility>
#include <algorithm>
#include "StringGraph.h"
#include "WorkSpace.h"


using namespace std;

namespace MathPack::MatrixEqu {

    void solveSLAU(vector<vector<double>> const& mA, vector<double> const& B, vector<double> &result);

    /**
     * Euqlid norm of vecrot inp
     * @param inp
     * @return
     */
    double norm(vector<double> const& inp);

    template <typename T> pair<size_t, size_t> findFirst(vector<vector<T>> const& matrx, T const& val);

    /**
     * Euqlid norm of vector difference inp1-inp2
     * @param inp1
     * @param inp2
     * @return
     */
    double normOfDiffer(vector<double> const& inp1, vector<double> const& inp2);

    vector<vector<double>> parseSLAU(vector<vector<double>> const& A, vector<vector<double>> const& b);

    vector<double> mulMatxToRow(vector<vector<double>> const& A, vector<double> const& B);

    vector<shared_ptr<StringGraph>> mulMatxToRow_s(vector<vector<shared_ptr<StringGraph>>> A, vector<shared_ptr<StringGraph>> B);

    vector<shared_ptr<StringGraph>> mulMatxToRow_s(vector<vector<double>> A, vector<shared_ptr<StringGraph>> B);

    /**
     * Determinat of integer matrix
     * @param A integer matrix
     * @return det(A)
     */
    double det_i(vector<vector<int>> A);

    /**
     * Determinant
     * @param A double matrix
     * @return det(A)
     */
    double det(vector<vector<double>> A);

    /**
     * Determinant
     * @param A symbol matrix
     * @return det(A)
     */
    shared_ptr<StringGraph> det_s(vector<vector<shared_ptr<StringGraph>>> A);

    /**
     * VERY IMPORTANT! Answer will be returned in b
     * @param A
     * @param b
     * @return
     */
    void solveLU(vector<vector<double>> A, vector<double> &b);

    /**
     * VERY IMPORTANT! Answer will be returned in b
     * @param A
     * @param b
     */
    void solveLU(vector<vector<int>> A, vector<shared_ptr<StringGraph>> &b);

    /**
     * Converts integer matrix to double
     * @param inp integer matrix
     * @return double matrix
     */
    vector<vector<double>> int2dbl(vector<vector<int>> inp);

    vector<vector<double>> mul(vector<vector<double>> A, vector<vector<double>> B);

    vector<double> getColumn(vector<vector<double>> A, size_t i);

    /**
     * Multiplex matrix to scalar
     * @param matrixB -matrix
     * @param d - scalar
     * @return matrix
     */
    vector<vector<double>> mulElWise(vector<vector<double>> matrixB, double d);

    /**
     * Multiplex matrix to scalar
     * @param matrixB -matrix
     * @param d - scalar
     * @return matrix
     */
    vector<vector<shared_ptr<StringGraph>>> mulElWise_s(vector<vector<shared_ptr<StringGraph>>> matrixB, shared_ptr<StringGraph> d);

    vector<vector<double>> pinv(vector<vector<double>> input);

    vector<vector<double>> invMatr(vector<vector<double>> A);

    vector<vector<shared_ptr<StringGraph>>> invMatr_s(vector<vector<shared_ptr<StringGraph>>> input);

    vector<vector<double>> transpose(vector<vector<double>> input);

    vector<vector<shared_ptr<StringGraph>>> transpose_s(vector<vector<shared_ptr<StringGraph>>> input);

    vector<vector<double>> adjMatr(vector<vector<double>> inp);

    vector<vector<shared_ptr<StringGraph>>> adjMatr_s(vector<vector<shared_ptr<StringGraph>>> inp);

    /**
     * Index starts from 0.
     * @param row
     * @return ind of solo elem or -1, if elems more than one.
     */
    int getSingleInd(vector<int> const& row);

    /**
     * for +/- 1 matrix only
     * @param system
     * @param ans
     * @return
     */
    vector<shared_ptr<StringGraph>> solve(vector<vector<int>> system, vector<shared_ptr<StringGraph>> ans);

    vector<vector<double>> evalSymbMatr(vector<vector<shared_ptr<StringGraph>>> const& inp);

    /**
     * Evaluates and puts inp's values into out.
     * @param out
     * @param inp
     * @param vars
     * @param extInput
     */
    void putValuesFromSymbMatr(vector<vector<double>> &out, vector<vector<shared_ptr<StringGraph>>> const& inp);

    vector<double> evalSymbRow(vector<shared_ptr<StringGraph>> inp);

    void putValuesFromSymbRow(vector<double> &out, vector<shared_ptr<StringGraph>> const& inp);

    size_t rank(vector<vector<int>> inp);
    size_t rank(vector<vector<int>> & A, vector<shared_ptr<StringGraph>> & b);
}

#endif

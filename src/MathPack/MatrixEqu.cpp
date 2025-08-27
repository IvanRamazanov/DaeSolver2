/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "MatrixEqu.h"
#include <cstring>

/**
 *
 * @author Ivan
 */
namespace MathPack::MatrixEqu {
    /**
     * validates mextris size is square
     * @return size of the matrix
     */
    template <typename T> size_t validate_matrix_is_square(vector<vector<T>> const& mtx){
        size_t size_row = mtx.size();
        if (size_row > 0){
            if (mtx[0].size() != size_row){
                // Note: works on assumption that all columns are equal size
                throw invalid_argument("Matrix is not square!");
            }
        }
        return size_row;
    }

    template <typename T> size_t find_elem(vector<T> const& vec, T const& value){
        auto pos = find(vec.begin(), vec.end(), value);
        if (pos == vec.end()){
            return -1;
        }
        return distance(vec.begin(), pos);
    }

    template <typename T> size_t find_last_elem(vector<T> const& vec, T const& value){
        auto pos = find(vec.rbegin(), vec.rend(), value);
        if (pos == vec.rend()){
            return -1;
        }
        return distance(pos, vec.rend())-1;
    }

    vector<vector<double>> solveSLAU(vector<vector<double>> mA, vector<vector<double>> B) {
        //TODO: why B is [m,1] matrix? Shound't it be a vector?

        size_t size_A = validate_matrix_is_square(mA);
        if (size_A == 0){
            throw invalid_argument("Matrix is empty!");
        }
        if (B.size() != size_A){
            throw invalid_argument("B should be the same length as mA!");
        }
        if (B[0].size() != 1){
            throw invalid_argument("B should be a vector!");
        }

        for(size_t i=0; i<size_A; i++){
            if(mA[i][i] == 0){
                for(size_t j=i+1; j<size_A; j++){
                    if(mA[j][i]!=0){
                        for(size_t k=0; k<size_A; k++){
                            // swap i,k and j,k values
                            double temp = mA[i][k];
                            mA[i][k] = mA[j][k];
                            mA[j][k] = temp;
                        }
                        double temp=B[i][0];
                        B[i][0] = B[j][0];
                        B[j][0] = temp;
                        break;
                    }
                }
            }
            //solver
            for(size_t j=0; j<size_A; j++){
                double k;
                vector<double> row = mA[i]; //row.addAll(mA.get(i));
                row.push_back(B[i][0]);
                if(i != j){
                    k = -1.0 * mA[j][i]/mA[i][i];
                    if(k != 0.0){
                        // TODO: too precise? Use epsilon?
                        for(size_t m=0; m<row.size(); m++){
                            row[m] = row[m]*k;
                            if(m < size_A){
                                mA[j][m] += row[m];
                            }else{
                                B[j][0] += row[m];
                            }
                        }
                    }
                }else{
                    k = 1.0/mA[j][i];
                    if(k != 1.0){
                        // TODO: too precise? Use epsilon?
                        for(size_t m=0; m<row.size(); m++){
                            if(m < size_A){
                                mA[j][m] *= k;
                            }
                            else{
                                B[j][0] *= k;
                            }
                        }
                    }
                }
            }
        }

        //vector<vector<double>> output;
        //for(auto data:B){
        //    output.push_back(vector<double>{data});
        //}
        //return(output);
        return B;
    }

    /**
     * Euqlid norm of vecrot inp
     * @param inp
     * @return
     */
    double norm(vector<double> const& inp){
        double out=0;
        for(double x:inp){
            out += x*x;
        }
        out = sqrt(out);
        return out;
    }

    template <typename T> pair<size_t, size_t> findFirst(vector<vector<T>> const& asd, T const& val){
        for(size_t i=0; i<asd.size(); i++){
            if(find_elem(asd[i], val) != -1){
                return make_pair<size_t, size_t>(i, find_last_elem(asd[i], val));
            }
        }
        return make_pair<size_t, size_t>(-1, -1);
    }

    /**
     * Euqlid norm of vector difference inp1-inp2
     * @param inp1
     * @param inp2
     * @return
     */
    double normOfDiffer(vector<double> const& inp1, vector<double> const& inp2){
        double out = 0;
        for(size_t i=0; i<inp1.size(); i++){
            double add = pow((inp1[i]-inp2[i])/inp1[i], 2);
            if(isnan(add))
                out+=0;
            else
                out+=add;
        }
        out = sqrt(out);
        return out;
    }

    vector<vector<double>> parseSLAU(vector<vector<double>> const& A, vector<vector<double>> const& b) {
        // TODO: does b really needs to be a matrix?

        size_t a_size = validate_matrix_is_square(A);

        vector<bool> blacklist(a_size, false);
        vector<vector<double>> out(a_size, vector<double>(1, 0.0));
        vector<vector<double>> mA = A;
        vector<vector<double>> B = b;
        vector<double>* niceRow = NULL;

        while(true){
            if(mA.empty()){
                break;
            }

            niceRow = NULL;
            for(auto& row : mA){
                //get non zeros
                int a=0;
                for(auto& elem : row){
                    if(elem != 0.0){
                        a++;
                    }
                }
                if(a == 1){
                    niceRow = &row;
                    break;
                }
            }
            if(niceRow == NULL){
                for(int i=blacklist.size()-1; i>=0; i--){
                    if(blacklist[i]){
                        for(size_t j=0; j<mA.size(); j++){
                            mA[j].erase(mA[j].begin()+i);
                        }
                    }
                }

                //---RESOLVING BAD INIT CONDS-----------(Not final version)
                vector<vector<double>> res = solveSLAU(mA, B);
                int k=0;
                for(size_t i=0; i<blacklist.size(); i++){
                    if(!blacklist[i]){
                        out[i][0] = res[k][0];
                        k++;
                    }
                }
                break;
            }else{
                int ind = find_elem(mA, *niceRow);
                int subInd=0;
                for(auto& a : *niceRow){
                    if(a != 0){
                        break;
                    }else{
                        subInd++;
                    }
                }
                out[subInd][0] = B[ind][0]/mA[ind][subInd];
                blacklist[subInd] = true;
                (*niceRow)[subInd] = 0.0;
                for(int i=mA.size()-1; i>=0; i--){
                    vector<double> row = mA[i];
                    double k = row[subInd]*out[subInd][0];
                    row[subInd] = 0.0;
                    B[find_elem(mA, row)][0] -= k;
                    int a=0;
                    for(auto& val:row){
                        if(val != 0.0){
                            a++;
                        }
                    }
                    if(a == 0.0){
                        //if(B.get(mA.indexOf(row)).get(0)!=0.0){

                        //}else{
                            //should be here
                        //}
                        B.erase(B.begin() + i);
                        mA.erase(mA.begin() + i); //these ones
                    }
                }
            }

        }
        return(out);
    }

    vector<double> mulMatxToRow(vector<vector<double>> const& A, vector<double> const& B){
        size_t m_size = validate_matrix_is_square(A);
        if (B.size() != m_size){
            throw invalid_argument("mulMatxToRow::incompatible matrix sizes: B should be same size as A");
        }

        vector<double> out(m_size);
        for(size_t i=0; i<m_size; i++){
            double sum=0;
            for(size_t j=0; j<m_size; j++){
                sum += A[i][j] * B[j];
            }
            out[i] = sum;
        }
        return out;
    }

    vector<shared_ptr<StringGraph>> mulMatxToRow_s(vector<vector<shared_ptr<StringGraph>>> A, vector<shared_ptr<StringGraph>> B){
        vector<shared_ptr<StringGraph>> out;
        for(size_t i=0; i<A.size(); i++){
            if(!A[i].empty()){
                vector<node_container<Node>> inps;
                vector<int> gains;
                for(size_t j=0; j<B.size(); j++){
                    inps.push_back(make_node<FuncNode>("*", A[i][j]->getRoot()->copy(), B[j]->getRoot()->copy()));
                    gains.push_back(1);
                }
                out.push_back(make_shared<StringGraph>(make_node<FuncNode>("+",inps,gains)));
            }else{
                out.push_back(nullptr);
            }
        }
        return out;
    }

    vector<shared_ptr<StringGraph>> mulMatxToRow_s(vector<vector<double>> A, vector<shared_ptr<StringGraph>> B){
        vector<shared_ptr<StringGraph>> out;
        for(size_t i=0;i<A.size();i++){
            if(!A[i].empty()){
                vector<node_container<Node>> inps;
                vector<int> gains;
                for(size_t j=0; j<B.size(); j++){
                    inps.push_back(make_node<FuncNode>("*", make_node<Const>(A[i][j]), B[j]->getRoot()->copy()));
                    gains.push_back(1);
                }
                out.push_back(make_shared<StringGraph>(make_node<FuncNode>("+", inps, gains)));
            }else{
                out.push_back(nullptr);
            }
        }
        return out;
    }

    /**
     * Determinat of integer matrix
     * @param A integer matrix
     * @return det(A)
     */
    double det_i(vector<vector<int>> A){
        size_t numOfRows=A.size(),
            numOfCols = A.at(0).size();
        double result;
        switch (numOfRows) {
            case 1:
                result=A.at(0).at(0);
                break;
            case 2:
                result=A.at(0).at(0)*A.at(1).at(1)-A.at(0).at(1)*A.at(1).at(0);
                break;
            //general case
            default:
                result=0;
                for(size_t i=0; i<numOfCols; i++){
                    vector<vector<int>> M;
                    for(size_t j=1; j<numOfRows; j++){
                        M.push_back(A.at(j));
                        M.at(j-1).erase(M.at(j-1).begin() + i);
                    }
                    if(A.at(0).at(i) != 0.0){
                        result+=pow(-1.0, i)*A.at(0).at(i)*det_i(M);
                    }else{
                        result+=0.0;
                    }
                }
                break;
        }
        return result;
    }

    /**
     * Determinant
     * @param A double matrix
     * @return det(A)
     */
    double det(vector<vector<double>> A){
        size_t numOfRows=A.size(),
            numOfCols=A.at(0).size();
        double result;
        switch (numOfRows) {
            case 1:
                result=A.at(0).at(0);
                break;
            case 2:
                result=A.at(0).at(0)*A.at(1).at(1)-A.at(0).at(1)*A.at(1).at(0);
                break;
            //general case
            default:
                result=0;
                for(size_t i=0;i<numOfCols;i++){
                    vector<vector<double>> M(numOfRows-1);
                    for(size_t j=1; j<numOfRows; j++){
                        M.push_back(A.at(j));
                        M.at(j-1).erase(M.at(j-1).begin() + i);
                    }
                    if(A.at(0).at(i)!=0.0){
                        result+=pow(-1.0, i)*A.at(0).at(i)*det(M);
                    }else{
                        result+=0.0;
                    }
                }
                break;
        }
        return result;
    }

    /**
     * Determinant
     * @param A symbol matrix
     * @return det(A)
     */
    shared_ptr<StringGraph> det_s(vector<vector<shared_ptr<StringGraph>>> A){
        size_t numOfRows=A.size(),
            numOfCols=A.at(0).size();
        if(numOfCols!=numOfRows) 
            throw new runtime_error("Matrix dimensions missmatch!");
        shared_ptr<StringGraph> result = nullptr;
        switch (numOfRows) {
            case 1:
                result=make_shared<StringGraph>(*A.at(0).at(0));
                break;
            case 2:
                result=StringGraph::sum(StringGraph::mul(A.at(0).at(0), A.at(1).at(1), 1),
                                        StringGraph::mul(A.at(0).at(1),A.at(1).at(0), 1),
                                        -1);
                break;
            //general case
            default:
                result=make_shared<StringGraph>(0);
                for(size_t i=0; i<numOfCols; i++){
                    vector<vector<shared_ptr<StringGraph>>> M(numOfRows-1);
                    for(size_t j=1; j<numOfRows; j++){
                        M.push_back(vector<shared_ptr<StringGraph>>(A.at(j)));
                        M.at(j-1).erase(M.at(j-1).begin() + i);
                    }
                    if(A.at(0).at(i)->getRoot()->type() == ConstNode){
                        if(cast_node<Const>(A.at(0).at(i)->getRoot())->getValue() != 0.0){
                            double k=1.0-2.0*((i)%2);
                            result->add(StringGraph::mul(
                                StringGraph::mul(A.at(0).at(i), det_s(M), 1),
                                (int) k
                                ));
                        }
                    }else{
                        double k=1.0-2.0*((i)%2);
                        result->add(StringGraph::mul(StringGraph::mul(A.at(0).at(i), det_s(M), 1), (int) k));
                    }

                }
                break;
        }
        return result;
    }

    /**
     * VERY IMPORTANT! Answer will be returned in b
     * @param A
     * @param b
     * @return
     */
    void solveLU(vector<vector<double>> A, vector<double>& b){
        size_t a_size = validate_matrix_is_square(A);
        if (b.size() != a_size){
            throw invalid_argument("solveLU::matrix A should be the same size as vector b");
        }

        int n=A.size(),
            i=0,
            index;
        vector<vector<double>> L(n),
                    U(n);
        for (int i=0; i<n; i++){
            L[i] = vector<double>(n);
            //U.push_back(vector<double>(n));
        }

        //init
        for(auto& row:A) {
            U[i] = row;
            i++;
        }

        // main cycle for rows
        for(i=0; i<n; i++){
            // permutation
            double max = abs(U[i][i]);
            index=i;
            for(int j=i+1;j<n;j++){
                double val = abs(U[j][i]);
                if(val>max){
                    max=val;
                    index=j;
                }
            }
            if(index!=i){
                // swap
                for(int j=i;j<n;j++){
                    double tmp=U[i][j];
                    U[i][j]=U[index][j];
                    U[index][j]=tmp;
                }
                double tmp=b[i];
                b[i]=b[index];
                b[index]=tmp;
            }


            // general cycle
            L[i][i]=1.0;
            for(int j=i+1;j<n;j++){
                double k=U[j][i]/U[i][i];

                // change L
                L[j][i]=k;

                // subtract rows
                for(int m=i;m<n;m++){
                    U[j][m]=U[j][m]-U[i][m]*k;
                }

                // ans
                b[j]=b[j]-b[i]*k;
            }
        }

        // solve LY=b
        vector<double> Y(n);
//        for(i=0;i<n;i++){
//            Y[i]=b[i];
//            for(int j=i+1;j<n;j++){
//                b[j]=b[j]-Y[i]*L[j][i];
//            }
//        }

        // solve UX=Y
        for(i=n-1;i>=0;i--){
//            Y[i]=Y[i]/U[i][i];
            b[i]=b[i]/U[i][i];
            for(int j=i-1;j>=0;j--){
                b[j]=b[j]-b[i]*U[j][i];
            }
        }
//        return Y;
    }

    /**
     * VERY IMPORTANT! Answer will be returned in b
     * @param A
     * @param b
     */
    void solveLU(vector<vector<int>> A, vector<shared_ptr<StringGraph>> &b){
        if(A.size()!=A.at(0).size()) 
            throw runtime_error("Matrix must be square!");
        int n=A.size(),i=0,index;
        vector<vector<double>> L(n),
            U(n);
        for (int i=0; i<n; i++){
            L[i] = vector<double>(n);
            U[i] = vector<double>(n);
        }

        //init
        for(auto& row:A) {
            for(size_t j=0; j<row.size(); j++) {
                int val = row[j];
                U[i][j] = (double) val;
            }
            i++;
        }

        // main cycle for rows
        for(i=0; i<n; i++){
            // permutation
            double max=abs(U[i][i]);
            index=i;
            for(int j=i+1;j<n;j++){
                double val=abs(U[j][i]);
                if(val>max){
                    max=val;
                    index=j;
                }
            }
            if(index!=i){
                // swap
                for(int j=i;j<n;j++){
                    double tmp=U[i][j];
                    U[i][j]=U[index][j];
                    U[index][j]=tmp;
                }
                auto tmp=b.at(i);
                b[i] = b.at(index);
                b[index] = tmp;
            }


            // general cycle
            L[i][i]=1.0;
            for(int j=i+1; j<n; j++){
                double k=U[j][i]/U[i][i];

                // change L
                L[j][i]=k;

                // subtract rows
                for(int m=i;m<n;m++){
                    U[j][m]=U[j][m]-U[i][m]*k;
                }

                // ans
                b.at(j)->sub(StringGraph::mul(b.at(i), k));
            }
        }

        // solve UX=Y
        for(i=n-1; i>=0; i--){
//            Y[i]=Y[i]/U[i][i];
            b.at(i)->multiply(1.0/U[i][i]);
            for(int j=i-1; j>=0; j--){
//                b[j]=b[j]-b[i]*U[j][i];
                b.at(j)->sub(StringGraph::mul(b.at(i), U[j][i]));
            }
        }

        for(i=0; i<b.size(); i++){
            if(b.at(i)->getRoot()->type() == FunctionNode)
                b.at(i)->expand();
        }
    }

    /**
     * Converts integer matrix to double
     * @param inp integer matrix
     * @return double matrix
     */
    vector<vector<double>> int2dbl(vector<vector<int>> inp){
        vector<vector<double>> out;
        int i=0;
        for(auto& row:inp){
            out.push_back(vector<double>());
            for(auto& val:row){
                out.at(i).push_back(val);
            }
            i++;
        }
        return out;
    }

    vector<vector<double>> mul(vector<vector<double>> A, vector<vector<double>> B) {
        vector<vector<double>> output;
        for(size_t i=0; i<A.size(); i++){
            vector<double> row;
            for(size_t j=0; j<B.at(0).size(); j++){
                double c=0;
                for(size_t k=0; k<A.at(0).size(); k++){
                    c=c+A.at(i).at(k)*B.at(k).at(j);
                }
                row.push_back(c);
            }
            output.push_back(row);
        }
        return(output);
    }

    /**
     * Multiplex matrix to scalar
     * @param matrixB -matrix
     * @param d - scalar
     * @return matrix
     */
    vector<vector<double>> mulElWise(vector<vector<double>> matrixB, double d) {
        vector<vector<double>> output;
        for(auto& data:matrixB){
            vector<double> row;
            for(auto& element:data){
                row.push_back(element*d);
            }
            output.push_back(row);
        }
        return(output);
    }

    vector<double> getColumn(vector<vector<double>> A, size_t i){
        vector<double> out(A.size());
        for(size_t j=0;j<A.size();j++)
            out[j] = A.at(j).at(i);
        return out;
    }

    /**
     * Multiplex matrix to scalar
     * @param matrixB -matrix
     * @param d - scalar
     * @return matrix
     */
    vector<vector<shared_ptr<StringGraph>>> mulElWise_s(vector<vector<shared_ptr<StringGraph>>> matrixB, shared_ptr<StringGraph> d) {
        vector<vector<shared_ptr<StringGraph>>> output;
        for(auto& data:matrixB){
            vector<shared_ptr<StringGraph>> row;
            for(auto& element:data){
                row.push_back(StringGraph::mul(element, d, 1));
            }
            output.push_back(row);
        }
        return(output);
    }

    vector<vector<double>> pinv(vector<vector<double>> input){
        return mul(invMatr(mul(transpose(input), input)), transpose(input));
    }

    // slow version (?)
    // vector<vector<double>> invMatr(vector<vector<double>> A) {
    //     int len=A.size();
    //     if(len!=A[0].size()){
    //         throw runtime_error("Mathris must be square!");
    //     }
    //     vector<vector<double>> dumpA(len);
    //     vector<vector<double>> E(len);
    //     for(int i=0;i<len;i++){
    //         dumpA[i] = A[i];
    //         E[i] = vector<double>(len);
    //         E[i][i]=1;
    //     }
    //     //----solver-------
    //     for(int i=0;i<len;i++){
    //         if(dumpA[i][i]==0){
    //             auto& tempA = dumpA[i];
    //             auto& tempE = E[i];
    //             for(int j=i+1;j<len;j++){
    //                 if(dumpA[j][i]!=0){
    //                     dumpA[i]=dumpA[j];
    //                     dumpA[j]=tempA;
    //                     E[i]=E[j];
    //                     E[j]=tempE;
    //                     break;
    //                 }
    //             }
    //         }
    //         if(dumpA[i][i]<0){
    //             for(int j=0;j<len;j++){
    //                 dumpA[i][j]=-1*dumpA[i][j];
    //                 E[i][j]= -1*E[i][j];
    //             }
    //         }

    //         //solver----
    //         vector<double> row(len*2);
    //         memcpy(row.data(), dumpA[i].data(), len);
    //         memcpy(row.data()+len, E[i].data(), len);
    //         for(int j=0; j<dumpA.size(); j++){
    //             double k;
    //             //row.addAll(dumpA.get(i));
    //             if(i!=j){
    //                 k=-1*dumpA[j][i]/row[i];
    //                 if(k!=1){
    //                     for(int m=0;m<row.size();m++){
    //                         if(m<dumpA[0].size())
    //                             dumpA[j][m]=k*row[m]+dumpA[j][m];
    //                         else
    //                             E[j][m-dumpA.size()]=row[m]*k+E[j][m-dumpA.size()];
    //                     }
    //                 }
    //             }else{
    //                 k=1/dumpA[j][i];
    //                 if(k!=1){
    //                     for(int m=0;m<row.size();m++){
    //                         if(m<dumpA.size())
    //                             dumpA[j][m]=dumpA[j][m]*k;
    //                         else
    //                             E[j][m-dumpA.size()]=E[j][m-dumpA.size()]*k;
    //                     }
    //                 }
    //             }
    //         }
    //         //??????????
    //     }
    //     //--------
    //     return(E);
    // }

    vector<vector<double>> invMatr(vector<vector<double>> A) {
        if(A.size() != A.at(0).size()){
            throw runtime_error("Matrix must be square!");
        } 
        if(A.size()==1){
            vector<vector<double>> out(1);
            out[0].push_back(1/A[0][0]);
            return out;
        }else{
            return mulElWise(adjMatr(A),1/det(A));
        }
    }

    vector<vector<shared_ptr<StringGraph>>> invMatr_s(vector<vector<shared_ptr<StringGraph>>> input) {
        if(input.size()==1 && input.at(0).size()==1){
            vector<vector<shared_ptr<StringGraph>>> out(1);
            out.at(0).push_back(StringGraph::divide(1, input[0][0]));
            return out;
        }else
            return mulElWise_s(adjMatr_s(input), StringGraph::divide(1, det_s(input)));
    }

    vector<vector<double>> transpose(vector<vector<double>> input){
        size_t m=input.size();
        if(m<1) throw runtime_error("Empty Matrix(NxM)! N must be greater than 0");
        size_t n=input.at(0).size();
        if(n<1) throw runtime_error("Empty Matrix(NxM)! M must be greater than 0");

        vector<vector<double>> out(n);
        for(size_t i=0; i<n; i++){
            out.push_back(vector<double>(m));
            for(size_t j=0; j<m; j++){
                out[i][j] = input.at(j).at(i);
            }
        }
        return out;
    }

    vector<vector<shared_ptr<StringGraph>>> transpose_s(vector<vector<shared_ptr<StringGraph>>> input){
        size_t m=input.size();
        if(m<1) throw runtime_error("Empty Matrix(NxM)! N must be greater than 0");
        size_t n=input.at(0).size();
        if(n<1) throw runtime_error("Empty Matrix(NxM)! M must be greater than 0");
        vector<vector<shared_ptr<StringGraph>>> out(n);
        for(size_t i=0; i<n; i++){
            out[i] = vector<shared_ptr<StringGraph>>(m);
            for(size_t j=0; j<m; j++){
                out.at(i).push_back(input.at(j).at(i));
            }
        }
        return out;
    }

    vector<vector<double>> adjMatr(vector<vector<double>> inp){
        vector<vector<double>> out(inp.size());
        size_t nRows=inp.size(),
            nCols=inp.at(0).size();
        if(nCols!=nRows)
            throw new runtime_error("Bad matrix! N != M");
        for(size_t i=0;i<nRows;i++){
            out.push_back(vector<double>(nCols));
            for(size_t j=0;j<nCols;j++){
                double k=1.0-2.0*((i+j)%2);
                vector<vector<double>> M(nRows);
                for(auto& row:inp) 
                    M.push_back(vector<double>(row));
                M.erase(M.begin() + i);
                for(size_t m=0;m<M.size();m++)
                    M[m].erase(M[m].begin() + j);
                out.at(i).push_back(k*det(M));
            }
//            System.out.println(out.get(i).toString());
        }
        return transpose(out);
    }

    vector<vector<shared_ptr<StringGraph>>> adjMatr_s(vector<vector<shared_ptr<StringGraph>>> inp){
        vector<vector<shared_ptr<StringGraph>>> out(inp.size());
        size_t nRows=inp.size(),
            nCols=inp.at(0).size();
        if(nCols!=nRows) 
            throw runtime_error("Bad matrix! N != M");
        for(size_t i=0; i<nRows; i++){
            out[i] = vector<shared_ptr<StringGraph>>(nCols);
            for(size_t j=0; j<nCols; j++){
                double k=1.0-2.0*((i+j)%2);
                vector<vector<shared_ptr<StringGraph>>> M;
                for(auto& row:inp)
                    M.push_back(vector<shared_ptr<StringGraph>>(row));
                M.erase(M.begin() + i);
                for(size_t m=0; m<M.size(); m++) 
                    M[m].erase(M[m].begin() + j);
                out.at(i).push_back(StringGraph::mul(det_s(M), (int) k));
            }
//            System.out.println(out.get(i).toString());
        }
        return transpose_s(out);
    }

    /**
     * Index starts from 0.
     * @param row
     * @return ind of solo elem or -1, if elems more than one.
     */
    int getSingleInd(vector<int> const& row){
        int cnt=0,
            indx=-1,
            j=0;
        for(auto& i:row){
            if(i != 0){
                cnt++;
                indx=j;
                if(cnt == 2){
                    return -1;
                }
            }
            j++;
        }
        if(cnt == 1){
            return indx;
        }else{
            return -1;
        }
    }

    /**
     * for +/- 1 matrix only
     * @param system
     * @param ans
     * @return
     */
    vector<shared_ptr<StringGraph>> solve(vector<vector<int>> system, vector<shared_ptr<StringGraph>> ans){
        // TODO: just modify ans?

        vector<shared_ptr<StringGraph>> answer;

        for(size_t i=0; i<system.size(); i++){
            if(system[i][i] == 0){
                for(size_t j=i+1; j<system.size(); j++){
                    if(system[j][i] != 0){
                        for(size_t k=0; k<system.size(); k++){
                            int temp=system[i][k];
                            system[i][k] = system[j][k];
                            system[j][k] = temp;
                        }
                        iter_swap(ans.begin()+i, ans.begin()+j);
                        break;
                    }
                }
            }
            //solver----
            for(size_t j=0; j<system.size(); j++){
                int k;
                vector<int> row(system[i]);
                if(i != j){
                    k = -1*system[j][i] / system[i][i];
                    if(k != 0.0){
                        for(size_t m=0; m<row.size(); m++){
                            row[m] *= k;
                            system[j][m] += row[m];
                        }
                        ans[j] = StringGraph::sum({ans[j], StringGraph::mul(ans[i], k)});
                    }
                }else{
                    k = 1/system[j][i];
                    if(k != 1){
                        for(size_t m=0; m<row.size(); m++){
                            system[j][m] *= k;
                        }
                        ans[j] = StringGraph::mul(ans[j], k);
                    }
                }
            }
            //--------
        }
        for(auto& data:ans){
            answer.push_back(make_shared<StringGraph>(*data));
        }
        return answer;
    }

    vector<vector<double>> evalSymbMatr(vector<vector<shared_ptr<StringGraph>>> const& inp){
        // TODO inefficient, replace (?)

        vector<vector<double>> out(inp.size(), vector<double>(inp[0].size()));

        for(size_t i=0; i<inp.size(); i++){
            size_t j = 0;
            for(auto& sg:inp[i]){
                out[i][j] = sg->evaluate();
                j++;
            }
        }
        return out;
    }

    /**
     * Evaluates and puts inp's values into out.
     * @param out
     * @param inp
     * @param vars
     * @param extInput
     */
    void putValuesFromSymbMatr(vector<vector<double>>& out, vector<vector<shared_ptr<StringGraph>>> const& inp){
        for(size_t i=0; i<inp.size(); i++){
            size_t j=0;
            for(auto& sg:inp[i]){
                out[i][j] = sg->evaluate();
                j++;
            }
        }
    }

    vector<double> evalSymbRow(vector<shared_ptr<StringGraph>> const& inp){
        // TODO inefficient, replace(?)
        vector<double> out(inp.size());
        for(auto& row : inp){
            out.push_back(row->evaluate());
        }
        return out;
    }

    void putValuesFromSymbRow(vector<double>& out, vector<shared_ptr<StringGraph>> const& inp){
        for(size_t i=0; i<inp.size(); i++){
            out[i] = inp[i]->evaluate();
        }
    }

    size_t rank(vector<vector<int>> A){
        int nRow = A.size(),
            nCol = A.at(0).size(),
            rnk = 0,
            offset = 0;
        int i=0;
        while(i<nRow && i+offset<nCol){ //main cycle
            if(A[i][i+offset]==0){
                bool flag=true;
                //swap lines
                while(i+offset<nCol&&flag) {
                    for (int j = i ; j < nRow; j++) { //cycle by rows under current
                        if (A[j][i + offset] != 0) {
                            if(i!=j)
                                for (int q = 0; q < nCol; q++) {
                                    int tmp = A[j][q];
                                    A[j][q] = A[i][q];
                                    A[i][q] = tmp;
                                }
                            flag=false;
                            break;
                        }
                    }
                    if(flag){
                        offset++;
                    }
                }
                if(flag){
                    break;
                }
            }
            for(int j=i+1;j<nRow;j++){ 
                //cycle by rows under current
                if(A[j][i+offset]!=0){
                    double k = A[j][i+offset] / A[i][i+offset];
                    for(int q=i;q<nCol;q++){ 
                        //cycle by length of row
                        A[j][q]=A[j][q]-A[i][q]*k;
                    }
                }
            }
            i++;
        }

        for(i=0;i<nRow;i++){
            for(int j=i;j<nCol;j++){
                if(A[i][j]!=0){
                    rnk++;
                    break;
                }
            }
        }
        return rnk;
    }

    size_t rank(vector<vector<int>> & A, vector<shared_ptr<StringGraph>> & b){
        int nRow = A.size(),
            nCol = A.at(0).size(),
            rnk = 0,
            offset = 0;
        int i=0;
        while(i<nRow && i+offset<nCol){ //main cycle
            if(A[i][i+offset] == 0){
                bool flag = true;
                //swap lines
                while(i+offset<nCol && flag) {
                    for (int j = i ; j < nRow; j++) { //cycle by rows under current
                        if (A[j][i + offset] != 0) {
                            if(i!=j){
                                for (int q = 0; q < nCol; q++) {
                                    int tmp = A[j][q];
                                    A[j][q] = A[i][q];
                                    A[i][q] = tmp;
                                }
                            }
                            auto tmpSg = b[j];
                            b[j] = b[i];
                            b[i] = tmpSg;
                            flag=false;
                            break;
                        }
                    }
                    if(flag){
                        offset++;
                    }
                }
                if(flag){
                    break;
                }
            }
            for(int j=i+1; j<nRow; j++){ 
                //cycle by rows under current
                if(A[j][i+offset] != 0){
                    double k = A[j][i+offset] / A[i][i+offset];
                    for(int q=i; q<nCol; q++){ 
                        //cycle by length of row
                        A[j][q] = A[j][q]-A[i][q]*k;
                    }

                    // apply changes to b
                    b[j]->sub(StringGraph::mul(b[i], k));
                }
            }
            i++;
        }

        for(i=0; i<nRow; i++){
            for(int j=i;j<nCol;j++){
                if(A[i][j]!=0){
                    rnk++;
                    break;
                }
            }
        }
        return rnk;
    }
}


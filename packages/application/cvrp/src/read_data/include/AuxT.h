/* ****************************************
 * ****************************************
 *  Data:    19/09/24
 *  Arquivo: AuxT.h
 * ****************************************
 * ****************************************/

#ifndef DW_DECOMP_AUX_H
#define DW_DECOMP_AUX_H

#include <cstring>
#include <numeric>
#include <algorithm>
#include "safe_vector.h"
#include "sefe_array.h"
#include <tuple>

//#include <Eigen/Eigen>

#define __PRETTYFILE__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define PRINT_DEBUGG(inicio, texto) std::cout<<inicio<<"DEBUG: "<<texto<<"  FILE: "<<__PRETTYFILE__<<"  FUNC: "<<__PRETTY_FUNCTION__<<"  LINHA: "<<__LINE__<<"\n";

#define assertm(exp, msg) if(exp){std::cout<<msg<<"\n\nLINE: "<<__LINE__<<"\nFILE: "<<__PRETTYFILE__<<"\nFUNC: "<<__PRETTY_FUNCTION__<<"\n\n"; throw "ERROR";}



template<typename T>
void applyPermutation(Vector<T> &vet, const std::vector<int> &permutation, const size_t tam)
{

    static std::vector<bool> vetDone(vet.size());
    if(vetDone.size() < tam)
        vetDone.resize(tam);

    for(size_t i=0; i < tam; ++i)
        vetDone[i] = false;

    for(size_t i=0; i < tam; ++i)
    {
        if(vetDone[i])
            continue;

        vetDone[i] = true;
        size_t prev_j = i;
        size_t j = permutation[i];

        while(i != j)
        {
            std::swap(vet[prev_j], vet[j]);
            vetDone[j] = true;
            prev_j = j;
            j = permutation[j];
        }
    }

}


/**
 * Sort vet1 utilizando valores de vet0
 */
template<typename T>
void sortDoisVets(Vector<T> &vet0, VectorI &vet1, const size_t tam, const bool decresente)
{
    static std::vector<int> indices(vet0.size());
    if(indices.size() < tam)
        indices.resize(tam);


    std::iota(indices.begin(), indices.begin()+tam, 0);

    if(decresente)
        std::sort(indices.begin(), indices.begin()+tam, [&](size_t a, size_t b)->bool{return vet0[a] > vet0[b];});
    else
        std::sort(indices.begin(), indices.begin()+tam, [&](size_t a, size_t b)->bool{return vet0[a] < vet0[b];});

    applyPermutation(vet0, indices, tam);
    applyPermutation(vet1, indices, tam);
}

template<typename T, size_t TAM>
std::tuple<int,double> getMinArray(const Array<T,TAM> &vet, int tam)
{
    auto tuple = std::make_tuple(0, vet[0]);

    for(int i=1; i < tam; ++i)
    {
        if(vet[i] < get<1>(tuple))
        {
            get<0>(tuple) = i;
            get<1>(tuple) = vet[i];
        }
    }

    return std::move(tuple);
}


inline __attribute__((always_inline))
bool doubleEqual(double a, double b, double ep=std::numeric_limits<double>::epsilon())
{
    return std::fabs(a-b) < ep;
}

inline __attribute__((always_inline))
bool doubleLess(double a, double b, double ep=1E-3)
{
    return (a-b) <-ep;
}

/*
template<typename T, int option>
class TempSpMatPrint
{
public:

    Eigen::SparseMatrix<T,option> &m;
    TempSpMatPrint(Eigen::SparseMatrix<T,option> &m_):m(m_)
    {}

};


template<typename T, int option>
class TempSpVetPrint
{
public:

    Eigen::SparseVector<T,option> &m;
    TempSpVetPrint(Eigen::SparseVector<T,option> &m_):m(m_)
    {}

};


template<typename T, int option>
std::ostream & operator << (std::ostream & s, const TempSpMatPrint<T,option> &temp)
{
    s<<static_cast<const Eigen::SparseMatrixBase<Eigen::SparseMatrix<T,option>>&>(temp.m);
    return s;
}


template<typename T, int option>
std::ostream & operator << (std::ostream & s, const TempSpVetPrint<T,option> &temp)
{
    s<<static_cast<const Eigen::SparseMatrixBase<Eigen::SparseVector<T,option>>&>(temp.m);
    return s;
}
*/

template<typename T>
std::string printVet(const Vector<T> &vet, int tam)
{
    std::string str;
    for(int i=0; i < tam; ++i)
        str += std::to_string(vet[i]) + " ";

    return str;
}



#endif //DW_DECOMP_AUX_H

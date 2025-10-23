/* ****************************************
 * ****************************************
 *  Data:    06/11/24
 *  Arquivo: ConstrutivoBin.h
 *  Autor:   Igor de Andrade Junqueira
 *  Projeto: 2L-SDVRP
 * ****************************************
 * ****************************************/

#ifndef INC_2L_SDVRP_CONSTRUTIVOBIN_H
#define INC_2L_SDVRP_CONSTRUTIVOBIN_H

#include "Instancia.h"
#include "Solucao.h"

namespace ConstrutivoBinNS
{

    struct EpRot
    {
        int epId = 0;
        InstanciaNS::Rotacao r = InstanciaNS::Rot0;
        double atributo = 0.0;

        EpRot()=default;

        bool operator<(const EpRot &outro) const
        {
            return atributo < outro.atributo;
        }
    };

    bool canInsert(const SolucaoNS::Ponto &ep, const int itemId, const SolucaoNS::Bin &bin, InstanciaNS::Rotacao r);
    bool epColideItem(const SolucaoNS::Ponto &ep, const SolucaoNS::Ponto &ponto, const int itemId);

    int construtivoBinPacking(Vector<SolucaoNS::Bin> &vetBin,
                               const int vetBinTam,
                               const VectorI &vetItensC,
                               const int vetItensTam,
                               const double alpha);

    bool construtivoBinPacking(SolucaoNS::Bin &bin,
                               VectorI &vetItens,
                               const int vetItensTam,
                               const double alpha,
                               const int numRepeticoes);

}

#endif //INC_2L_SDVRP_CONSTRUTIVOBIN_H

/* ****************************************
 * ****************************************
 *  Data:    11/12/24
 *  Arquivo: IG.h
 *  Autor:   Igor de Andrade Junqueira
 *  Projeto: 2L-SDVRP
 * ****************************************
 * ****************************************/

#ifndef INC_2L_SDVRP_IG_H
#define INC_2L_SDVRP_IG_H

#include "Solucao.h"

namespace IgNs
{
    bool metaheuristicaIg(SolucaoNS::Solucao &solucao);

    inline __attribute__((always_inline))
    void rmRota(SolucaoNS::Solucao &sol, int r)
    {
        sol.distTotal -= sol.vetRota[r].distTotal;
        sol.vetRota[r].reset();
        sol.vetBin[r].reset();
    }
}

#endif //INC_2L_SDVRP_IG_H

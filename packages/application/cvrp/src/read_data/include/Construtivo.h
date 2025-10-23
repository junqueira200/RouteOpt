/* ****************************************
 * ****************************************
 *  Data:    27/11/24
 *  Arquivo: Construtivo.h
 *  Autor:   Igor de Andrade Junqueira
 *  Projeto: 2L-SDVRP
 * ****************************************
 * ****************************************/

#ifndef INC_2L_SDVRP_CONSTRUTIVO_H
#define INC_2L_SDVRP_CONSTRUTIVO_H

#include "Solucao.h"
#include "list"

namespace ConstrutivoNS
{
    struct Estado;

    struct Estado
    {
        int cliente    = -1;
        int rotaId     = -1;
        int pos        = -1;
        double incDist = 0.0;
        std::list<Estado>::iterator it;
        bool operator<(const Estado &estadoOutro) const
        {
            return incDist < estadoOutro.incDist;
        }

    };

    inline std::ostream& operator<<(std::ostream &os, const Estado &estado)
    {
        os << "[clie(" << estado.cliente << ") idVeic(" << estado.rotaId << ") pos(" << estado.pos << ") incDist(" << estado.incDist << ")]";
        return os;
    }

    bool construtivoVrp(SolucaoNS::Solucao &solucao, double alphaVrp, double alphaBin);
    bool todosClieAtendidos(const VectorD &vetClieCarga);
    void melhorInsercao(SolucaoNS::Rota &rota, int clie, int &pos, double &inc, double alphaBin);
    bool verificaInsercaoItensNoBin(const SolucaoNS::Bin &bin, int clie, double alphaBin);
    bool insereCandidato(SolucaoNS::Solucao &solucao, const Estado &estado, double alphaBin);
    inline bool estadoComp(const Estado &e0, const Estado &e1){return e0.incDist < e1.incDist;}

    inline const Estado estadoZero;

}

#endif //INC_2L_SDVRP_CONSTRUTIVO_H

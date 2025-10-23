/* ****************************************
 * ****************************************
 *  Data:    17/12/24
 *  Arquivo: BuscaLocal.h
 *  Autor:   Igor de Andrade Junqueira
 *  Projeto: 2L-SDVRP
 * ****************************************
 * ****************************************/

#ifndef INC_2L_SDVRP_BUSCALOCAL_H
#define INC_2L_SDVRP_BUSCALOCAL_H

#include "Solucao.h"
#include "Instancia.h"

using namespace InstanciaNS;

namespace BuscaLocalNS
{

    enum MV
    {   MvIntraRotaShifit,
        MvIntraRotaSwap,
        MvIntraRota2opt,
        MvInterRotasShifit,
        MvInterRotasSwap,
        MvInterRotasCross
    };

    constexpr int NumMv = 5;// 5;

    /** *******************************************************************************************
     *  *******************************************************************************************
     *  @param sol
     *  @param rotaR
     *  @param pos0
     *  @param pos1
     *
     *  *******************************************************************************************
     *  *******************************************************************************************/
    bool intraRotaShift(SolucaoNS::Solucao &sol, int rotaR, int pos0, int pos1);

    /** *******************************************************************************************
     *  *******************************************************************************************
     *  @param sol
     *  @param rotaR
     *  @param pos0
     *  @param pos1
     *
     *  *******************************************************************************************
     *  *******************************************************************************************/
    bool mvIntraRotaSwap(SolucaoNS::Solucao &sol, int rotaR, int pos0, int pos1);

    /** *******************************************************************************************
     *  *******************************************************************************************
     *  @param sol
     *  @param rotaR
     *  @param pos0
     *  @param pos1
     *
     *  *******************************************************************************************
     *  *******************************************************************************************/
    bool mvIntraRota2opt(SolucaoNS::Solucao &sol, int rotaR, int pos0, int pos1);

    /** *******************************************************************************************
     *  *******************************************************************************************
     *  @param sol
     *  @param alphaBin
     *  @param idRota0      Rota que recebera um cliente
     *  @param pos0         Posicao da rota0 que recebera um cliente (0 1 2 0; se pos0=0 => 0 * 1 2 0)
     *  @param idRota1      Rota que sera retirado o cliente
     *  @param pos1         Posicao em que sera retirado o clliente
     *
     *  *******************************************************************************************
     *  *******************************************************************************************/
    bool mvInterRotasShift(SolucaoNS::Solucao &sol, double alphaBin, int idRota0, int pos0, int idRota1, int pos1);


    /** *******************************************************************************************
     *  *******************************************************************************************
     *  @param sol
     *  @param alphaBin     Parametro para a heuristica de EP
     *  @param idRota0
     *  @param pos0         Posicao do cliente da rota0
     *  @param idRota1
     *  @param pos1         Posicao do cliente da rota1
     *
     *  *******************************************************************************************
     *  *******************************************************************************************/
    bool mvInterRotasSwap(SolucaoNS::Solucao &sol, double alphaBin, int idRota0, int pos0, int idRota1, int pos1);


    /** *******************************************************************************************
     *  *******************************************************************************************
     *  @param sol
     *  @param alphaBin     Parametro para a heuristica de EP
     *  @param idRota0
     *  @param pos0         Posicao do cliente da rota0
     *  @param idRota1
     *  @param pos1         Posicao do cliente da rota1
     *
     *  *******************************************************************************************
     *  *******************************************************************************************/
    bool mvInterRotasCross(SolucaoNS::Solucao &sol, double alphaBin, int idRota0, int pos0, int idRota1, int pos1);

    void rvnd(SolucaoNS::Solucao &sol);

    template<typename T>
    bool buscaIntraRota(SolucaoNS::Solucao &solucao, T Func(SolucaoNS::Solucao &sol, int rotaR, int pos0, int pos1))
    {

        for(int r=0; r < instanciaG.numVeiculos; ++r)
        {
            SolucaoNS::Rota &rota = solucao.vetRota[r];
            for(int pos0=0; pos0 < rota.numPos; ++pos0)
            {
                for(int pos1=0; pos1 < rota.numPos; ++pos1)
                {
                    if(Func(solucao, r, pos0, pos1))
                    {
                        std::string error;
                        if(!solucao.verificaSol(error))
                        {
                            std::cout<<"ERROR: "<<error<<"\n\n";
                            PRINT_DEBUGG("", "");
                            throw "ERROR";
                        }

                        return true;
                    }
                }
            }
        }

        return false;
    }


    template<typename T>
    bool buscaInterRotas(SolucaoNS::Solucao& solucao,
                         MV                  mv,
                         double              alphaBin,
                         T Func(SolucaoNS::Solucao &sol, double alphaBin, int idRota0, int pos0, int idRota1, int pos1))
    {
        std::string error;

        for(int r0=0; r0 < instanciaG.numVeiculos; ++r0)
        {
            SolucaoNS::Rota &rota0 = solucao.vetRota[r0];

            int r1Start = r0+1;
            if(mv == MvInterRotasShifit)
                r1Start = 0;

            for(int r1=r1Start; r1 < instanciaG.numVeiculos; ++r1)
            {
                if(r0 == r1)
                    continue;

                SolucaoNS::Rota &rota1 = solucao.vetRota[r1];

                for(int pos0=0; pos0 < rota0.numPos; ++pos0)
                {
                    for(int pos1=0; pos1 < rota1.numPos; ++pos1)
                    {
                        if(Func(solucao, alphaBin, r0, pos0, r1, pos1))
                        {
                            if(!solucao.verificaSol(error))
                            {
                                std::cout<<"ERROR: "<<error<<"\n\n";
                                PRINT_DEBUGG("", "");
                                throw "ERROR";
                            }
                            //std::cout<<"**************************Improve*******************\n\n";
                            //throw "NAO EH ERROR";
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }


}

#endif //INC_2L_SDVRP_BUSCALOCAL_H

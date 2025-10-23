/* ****************************************
 * ****************************************
 *  Data:    11/12/24
 *  Arquivo: IG.cpp
 *  Autor:   Igor de Andrade Junqueira
 *  Projeto: 2L-SDVRP
 * ****************************************
 * ****************************************/

#include "Ig.h"
#include "Construtivo.h"
#include "Instancia.h"
#include "InputOutput.h"
#include "rand.h"
#include "BuscaLocal.h"

using namespace SolucaoNS;
using namespace ConstrutivoNS;
using namespace InstanciaNS;
using namespace ParseInputNS;
using namespace RandNs;
using namespace BuscaLocalNS;

bool IgNs::metaheuristicaIg(SolucaoNS::Solucao &best)
{

    std::string strError;
    Solucao sol(instanciaG);
    int ultimaA = 0;

    for(int i=0; i < 500; ++i)
    {
        sol.reset();
        if(construtivoVrp(sol, input.alphaVrp, input.aphaBin))
        {
            if(!sol.verificaSol(strError))
            {
                std::cout<<"ERROR\n"<<strError<<"\n";
                throw "ERROR";
            }

            best.copiaSolucao(sol);
            break;
        }
    }

/*    bool ret = best.verificaSol(strError);
    if(!ret)
    {
        std::cout<<strError<<"\n\n";
        return false;
    }

    return true;*/


    if(!best.verificaSol(strError))
    {
        std::cout<<strError<<"\n\n";
        return false;
    }

    for(int i=0; i < input.numItIG; ++i)
    {
        for(int k=0; k < 2; ++k)
        {
            int r = getRandInt(0, instanciaG.numVeiculos-1);
            while(sol.vetRota[r].numPos == 2)
                r = (r+1)%instanciaG.numVeiculos;

            rmRota(sol, r);
        }


        if(!construtivoVrp(sol, input.alphaVrp, input.aphaBin))
            sol.copiaSolucao(best);
        else
        {
            //std::cout<<"Viavel!\n";
            rvnd(sol);

            if(sol.distTotal < best.distTotal)
            {
                best.copiaSolucao(sol);
                ultimaA = i;
            }

            else if(sol.distTotal > best.distTotal)
            {
                double gap = ((sol.distTotal-best.distTotal)/best.distTotal);
                if(gap > input.gapIgReset)
                    sol.copiaSolucao(best);
            }

            //std::cout<<"\t"<<sol.distTotal<<"\n\n";
        }

        //if((i%1000) == 0)
        //    std::cout<<"MELHOR SOL: "<<best.distTotal<<"\n\n";
    }

    //std::cout<<sol<<"\n\n";

    /*
    std::cout<<sol<<"\n\n";

    for(int i=1; i < instanciaG.numClientes; ++i)
    {
        Bin bin;
        bin.reset();


        #pragma GCC uroll 3
        for(int d=0; d < 3; ++d)
        {
            bin.binDim[d] = instanciaG.vetDimVeiculo[d];
        }

        bin.volumeTotal = 1;

        #pragma GCC uroll 3
        for(int d=0; d < 3; ++d)
        {
            bin.volumeTotal *= bin.binDim[d];

            if((d+1) == instanciaG.numDim)
                break;
        }

        for(int j=0; j < 1000; ++j)
        {
            bin.addEp(PontoZero);
            bool result = verificaInsercaoItensNoBin(bin, i, input.aphaBin);
            std::cout << "\tCliente(" << i << "): " << result << "\n";
            if(result)
                break;
        }
    }*/

    //std::cout<<"MELHOR SOL: "<<best.distTotal<<"\n\n";
    return sol.verificaSol(strError);
}
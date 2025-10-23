/* ****************************************
 * ****************************************
 *  Data:    27/11/24
 *  Arquivo: Construtivo.cpp
 *  Autor:   Igor de Andrade Junqueira
 *  Projeto: 2L-SDVRP
 * ****************************************
 * ****************************************/

#include "Construtivo.h"
#include "Instancia.h"
#include "ConstrutivoBin.h"
#include "rand.h"

using namespace InstanciaNS;
using namespace SolucaoNS;
using namespace ConstrutivoBinNS;
using namespace RandNs;

typedef std::list<ConstrutivoNS::Estado>::iterator EstadoIt;

bool ConstrutivoNS::construtivoVrp(SolucaoNS::Solucao &solucao, double alphaVrp, double alphaBin)
{

    /*
    std::cout<<"***********************************\n"<<
               "**********CONSTRUTIVO VRP**********\n\n";
    */

    // Indica a carga entrege ao cliente
    static VectorD vetClieCarga(instanciaG.numClientes);
    vetClieCarga.setAll(0.0);

    static VectorI vetClieAtend(instanciaG.numClientes);
    vetClieAtend.setAll(0);

    static Matrix<Estado*> matEstado(instanciaG.numVeiculos, instanciaG.numClientes);
    matEstado.setVal(nullptr);

    std::list<Estado> listaCand;


    // Calcula as demandas atendidas de cada cliente
    for(int veic=0; veic < instanciaG.numVeiculos; ++veic)
    {
        Rota &rota = solucao.vetRota[veic];

        for(int i=1; i < instanciaG.numClientes; ++i)
            vetClieCarga[i] += rota.vetDemClie[i];
    }


    for(int i=1; i < instanciaG.numClientes; ++i)
    {
        if(doubleEqual(vetClieCarga[i], instanciaG.vetDemandaCliente[i]))
            vetClieAtend[i] = 1;
    }

    int numIt = 0;


    do
    {

        // Prencher a tabela
        for(int rotaId=0; rotaId < instanciaG.numVeiculos; ++rotaId)
        {

            Rota &rota = solucao.vetRota[rotaId];

            if(doubleEqual(rota.binPtr->demandaTotal, instanciaG.veicCap))
                continue;


            for(int i=1; i < instanciaG.numClientes; ++i)
            {
                if(vetClieAtend[i])
                    continue;

                // Verifica se a entrada da mat esta preenchida
                if(matEstado.get(rotaId, i))
                    continue;

                // Verifica se a dem resultante eh maior que a capacidade do veiculo
                if((rota.binPtr->demandaTotal+instanciaG.vetDemandaCliente[i]) > instanciaG.veicCap)
                {
                    continue;
                }

                Estado estado;
                melhorInsercao(rota, i, estado.pos, estado.incDist, alphaBin);

                if(estado.pos == -1)
                    estado = estadoZero;
                else
                {
                    estado.rotaId  = rotaId;
                    estado.cliente = i;

                    listaCand.emplace_back(estado);
                    Estado *estadoPtr = &listaCand.back();
                    matEstado.get(rotaId, i) = estadoPtr;
                    estadoPtr->it = listaCand.end();
                    estadoPtr->it--;

                }

            } // End for(int i=1; i < instanciaG.numClientes; ++i)

        } // End for(int rotaId=0; rotaId < instanciaG.numVeiculos; ++rotaId)

        if(listaCand.empty())
            break;

        listaCand.sort();

        int size = std::max(1, int(alphaVrp * int(listaCand.size())));
        EstadoIt chosenIt = std::next(listaCand.begin(), getRandInt(0, (size-1)));
        Estado *chosenPtr = &(*chosenIt);

        /*
        std::cout<<"*********************************************\n"<<
                   "*********************************************\n"<<
                   "\nCandidato Escolhido: "<<*chosenPtr<<"\n";
        */

        if(!insereCandidato(solucao, *chosenPtr, alphaBin))
        {
            listaCand.erase(chosenPtr->it);
            matEstado.get(chosenPtr->rotaId, chosenPtr->cliente) = nullptr;
            continue;
        }

        vetClieCarga[chosenPtr->cliente] += instanciaG.vetDemandaCliente[chosenPtr->cliente];
        vetClieAtend[chosenPtr->cliente] = 1;

        // Remove todos os candidatos com o cliente que acabou de ser inserido
        int cliente = chosenPtr->cliente;
        int rotaId  = chosenPtr->rotaId;

        for(int i=0; i < instanciaG.numVeiculos; ++i)
        {
            if(matEstado(i, cliente))
            {
                listaCand.erase(matEstado(i, cliente)->it);
                matEstado.get(i, cliente) = nullptr;
            }
        }

        chosenPtr = nullptr;
        chosenIt = listaCand.end();


        // Apaga os candidatos da rota que foi alterada
        for(int j=1; j < instanciaG.numClientes; ++j)
        {
            if(matEstado(rotaId, j))
            {
                listaCand.erase(matEstado(rotaId, j)->it);
                matEstado.get(rotaId, j) = nullptr;
            }
        }


    }while(!todosClieAtendidos(vetClieCarga));


    return todosClieAtendidos(vetClieCarga);

}


void ConstrutivoNS::melhorInsercao(SolucaoNS::Rota &rota, int clie, int &pos, double &inc, double alphaBin)
{

    pos = -1;
    inc = INF_Double;

    if(!verificaInsercaoItensNoBin(*rota.binPtr, clie, alphaBin))
    {
        return;
    }

    for(int i=0; i < (rota.numPos-1); ++i)
    {
        double incI = -instanciaG.matDist(rota.vetRota[i], rota.vetRota[i+1]);
        incI += instanciaG.matDist(rota.vetRota[i], clie);
        incI += instanciaG.matDist(clie, rota.vetRota[i+1]);

        if(incI < inc)
        {
            inc = incI;
            pos = i;
        }
    }
}


bool ConstrutivoNS::todosClieAtendidos(const VectorD &vetClieCarga)
{

    for(int i=1; i < instanciaG.numClientes; ++i)
    {
        if(!doubleEqual(vetClieCarga[i], instanciaG.vetDemandaCliente[i]))
            return false;
    }

    return true;
}

bool ConstrutivoNS::verificaInsercaoItensNoBin(const SolucaoNS::Bin &bin, int clie, const double alphaBin)
{
    static Vector<Bin> binAux(1);
    copiaBin(bin, binAux[0]);

    static VectorI vetItens(instanciaG.maxNumItensPorClie);
    vetItens.setAll(-1);

    const int iniItem = instanciaG.matCliItensIniFim(clie, 0);
    const int fimItem = instanciaG.matCliItensIniFim(clie, 1);

    int tam = 0;

    for(int itemId=iniItem; itemId <= fimItem; ++itemId)
    {
        vetItens[tam] = itemId;
        tam += 1;
    }


    int resul = construtivoBinPacking(binAux, 1, vetItens, tam, alphaBin);

    //std::cout<<"\t\t\t\tRESU("<<resul<<")\n";

    return (tam == resul);

}

bool ConstrutivoNS::insereCandidato(SolucaoNS::Solucao &solucao, const Estado &estado, double alphaBin)
{

    static VectorI vetItens(instanciaG.maxNumItensPorClie);
    vetItens.setAll(-1);

    // Verifica a demanda
    Rota &rota = solucao.vetRota[estado.rotaId];
    double newDem = rota.binPtr->demandaTotal+instanciaG.vetDemandaCliente[estado.cliente];
    if(newDem > instanciaG.veicCap)
    {
        std::cout<<"ERROR\n Veiculo com demanda("<<newDem<<") maior que o permitido("<<instanciaG.veicCap<<")\n";
        PRINT_DEBUGG("", "");
        throw "ERROR";
    }

    // Verifica o incremento
    double inc = -instanciaG.matDist(rota.vetRota[estado.pos], rota.vetRota[estado.pos+1]);
    inc += instanciaG.matDist(rota.vetRota[estado.pos], estado.cliente);
    inc += instanciaG.matDist(estado.cliente, rota.vetRota[estado.pos+1]);

    if(!doubleEqual(inc, estado.incDist))
    {
        std::cout<<"ERROR\nIncremento de distancia no estado("<<estado.incDist<<") != calculado("<<inc<<")\n";
        PRINT_DEBUGG("", "");
        throw "ERROR";
    }

    // Escreve itens do cliente em vetItens
    const int iniItem = instanciaG.matCliItensIniFim(estado.cliente, 0);
    const int fimItem = instanciaG.matCliItensIniFim(estado.cliente, 1);

    int tam = 0;

    for(int itemId=iniItem; itemId <= fimItem; ++itemId)
    {
        vetItens[tam] = itemId;
        tam += 1;
    }


    // Tenta construir um empacotamento em ate 10 tentativas
    if(!construtivoBinPacking(*rota.binPtr, vetItens, tam, alphaBin, 10))
    {
       return false;
    }


    // Shifit rota
    for(int i=(rota.numPos-1); i > estado.pos; --i)
    {
        rota.vetRota[i+1] = rota.vetRota[i];
    }

    // Atualiza paramentros
    rota.vetRota[estado.pos+1] = estado.cliente;
    rota.numPos       += 1;
    //rota.demTotal     += instanciaG.vetDemandaCliente[estado.cliente];
    rota.distTotal    += estado.incDist;
    solucao.distTotal += estado.incDist;
    rota.vetDemClie[estado.cliente] = instanciaG.vetDemandaCliente[estado.cliente];

    return true;

}

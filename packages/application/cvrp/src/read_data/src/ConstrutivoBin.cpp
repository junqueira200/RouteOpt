/* ****************************************
 * ****************************************
 *  Data:    06/11/24
 *  Arquivo: ConstrutivoBin.cpp
 *  Autor:   Igor de Andrade Junqueira
 *  Projeto: 2L-SDVRP
 * ****************************************
 * ****************************************/

#include "ConstrutivoBin.h"
#include "sefe_array.h"
#include "AuxT.h"
#include "rand.h"
#include "InputOutput.h"
#include "BinPackingCP.h"

using namespace InstanciaNS;
using namespace SolucaoNS;
using namespace RandNs;
using namespace ParseInputNS;
using namespace BinPackingCP_NS;

bool ConstrutivoBinNS::canInsert(const Ponto &ep, const int itemId, const Bin &bin, Rotacao r)
{
    // Verifica se o item cabe no bin
    #pragma GCC unroll 3
    for(int d=0; d < 3; ++d)
    {
        if(ep.vetDim[d] + instanciaG.vetItens[itemId].vetDim[d] > bin.binDim[d])
            return false;

        if((d+1) == instanciaG.numDim)
            break;
    }

    // Verifica colisao com cada item que esta no bin
    for(int i=0; i < bin.numItens; ++i)
    {
        //if(pontosIguais(ep, bin.vetPosItem[i]))
        //    return false;

        //int numInterc = 0;
        int itemIdOutro = bin.vetItemId[i];

        // Verfica a intersecao em cada dimensao
        if(verificaColisaoDoisItens(itemIdOutro, itemId, bin.vetPosItem[i], ep, bin.vetRotacao[i], r))
            return false;
    }

    return true;
}


bool ConstrutivoBinNS::epColideItem(const SolucaoNS::Ponto &ep, const SolucaoNS::Ponto &ponto, const int itemId)
{                              //const double epX, const double epY, const double x, const double y, const int itemId

    static Array<double,2> arrayTemp0;

    // Verifica colisao nos eixos
    #pragma GCC unroll 3
    for(int d=0; d < 3; ++d)
    {
        arrayTemp0[0] = ponto.vetDim[d];
        arrayTemp0[1] = arrayTemp0[0] + instanciaG.vetItens[itemId].vetDim[d];

        if(!(ep.vetDim[d] > arrayTemp0[0] && ep.vetDim[d] < arrayTemp0[1]))
            return false;

        if((d+1) == instanciaG.numDim)
            break;
    }

    return true;

/*
    // Verifica colisao no eixo y
    arrayTemp0[0] = y;
    arrayTemp0[1] = y+instanciaG.vetItemAltura[itemId];

    if(epY >= arrayTemp0[0] && epY <= arrayTemp0[1])
        return true;
    else
        return false;

*/
}

// TODO: Armazenar a sequencia dos itens e a sua rotacao
int ConstrutivoBinNS::construtivoBinPacking(Vector<Bin> &vetBin,
                                            const int vetBinTam,
                                            const VectorI &vetItensC,
                                            const int vetItensTam,
                                            const double alpha)
{

    // TODO fix
    VectorI vetItens(vetItensC);

    if(PrintConst)
    {
std::cout << "\n************CONSTRUTIVO_BIN_PACKING************\n";
std::cout << "***********************************************\n\n";
    }

    static Vector<EpRot> vetIdEpRot(NumEpPorBin);

    static VectorI vetBinId(instanciaG.numVeiculos);
    static bool iniVetBinId = false;
    if(!iniVetBinId)
    {
        vetBinId.setAll(-1);
        iniVetBinId = false;
    }

    static VectorD vetBinVol(instanciaG.numVeiculos);  // Guarda a area restante
    static bool iniVetBinArea = false;
    if(!iniVetBinArea)
    {
        vetBinVol.setAll(INF_Double);
    }
    int tamVetBinVol = 0;
    int numItensAlocados = 0;

    VectorI seqItens(instanciaG.numItens);
    int tamseqItens = 0;

    // Tenta alocar todos os itens
    for(int k=0; k < vetItensTam; ++k)
    {
        int tamTemp = vetItensTam-k;//std::max(int(alpha*(vetItensTam-k)), 1);
        if(PrintConst)
std::cout<<"tamTemp: "<<tamTemp<<"; ((vetItensTam-k)): "<<vetItensTam-k<<"\n";
        int i = k;
        if(tamTemp > 1)
            i = getRandInt(k, (k+tamTemp-1));

        const int itemId = vetItens[i];
        seqItens[tamseqItens] = itemId;
        tamseqItens += 1;

        for(int t=i; t > k; --t)
            vetItens[t] = vetItens[t-1];

        if(PrintConst)
        {
std::cout << "\n***********************************************\n\n";
std::cout << "Item: " << itemId << "; " << instanciaG.vetItens[itemId].print() << "\n\n";
        }
        // Selecionar um bin
        tamVetBinVol = 0;
        for(int b=0; b < vetBinTam; ++b)
        {
            const Bin &bin = vetBin[b];

            if(bin.vazio())
                continue;

            double novoVolOcupado = bin.volumeOcupado + instanciaG.vetItens[itemId].volume;
            if(novoVolOcupado > bin.volumeTotal)
            {
                continue;
            }

            double novaDem = bin.demandaTotal + instanciaG.vetItens[itemId].peso;

            if(novaDem > instanciaG.veicCap)
                continue;

            double VolRestante = bin.volumeTotal - novoVolOcupado;
            vetBinId[tamVetBinVol] = b;
            vetBinVol[tamVetBinVol] = VolRestante;
            tamVetBinVol += 1;
        }

        // Verifica se nao existe bin
        if(tamVetBinVol == 0)
        {
            // Verifica se existe um bin vazio e o adiciona
            for(int b=0; b < vetBinTam; ++b)
            {
                Bin &bin = vetBin[b];

                if(!bin.vazio())
                    continue;

                if(PrintConst)
std::cout<<"\tAdd bin vazio\n\n";

                bin.addEp(PontoZero);

                vetBinId[0] = b;
                tamVetBinVol += 1;
                break;
            }
        }

        if(tamVetBinVol > 1)
        {
            sortDoisVets(vetBinVol, vetBinId, tamVetBinVol, true);
            if(PrintConst)
std::cout << "\tvetSort: " << vetBinId.printN(tamVetBinVol) << "\n\t         " << vetBinVol.printN(tamVetBinVol) << "\n\n";
        }

        bool realizouInsercao = false;

        // Percorre os bins
        for(int b=0; b < tamVetBinVol; ++b)
        {
            Bin &bin = vetBin[vetBinId[b]];
            if(PrintConst)
std::cout<<"\tBin escolhido: "<<vetBinId[b]<<"\n\n";

            // Verificar os EPs
            int numEps = 0;

            if(vetIdEpRot.size() < bin.numEps)
            {
                for(int k = (int)vetIdEpRot.size(); k < bin.numEps; ++k)
                {
                    vetIdEpRot.push_back(EpRot());
                }
            }

            if(PrintConst)
std::cout<<"\t\tNumEps: "<<bin.numEps<<"\n\n";

            // Percorre os EPs do bin
            for(int ep=0; ep < bin.numEps; ++ep)
            {

            if(PrintEP && PrintConst)
std::cout<<"\t\tChecando EP"<<bin.vetEp[ep].print()<<"\n";
                // Verifica a colisao do item colocado no EP

                Rotacao rotacao = Rotacao(getRandInt(0,1));
                if(canInsert(bin.vetEp[ep], itemId, bin, rotacao))
                {
                    if(PrintEP && PrintConst)
std::cout<<"\t\t\t"<<"Can insert\n";

                    vetIdEpRot[numEps].epId = ep;
                    vetIdEpRot[numEps].r    = rotacao;
                    vetIdEpRot[numEps].atributo = get<1>(getMinArray(bin.vetEp[ep].vetDim, instanciaG.numDim));

                    numEps += 1;
                }
                else
                {
                    rotacao = Rotacao((int(rotacao)+1)%2);
                    if(canInsert(bin.vetEp[ep], itemId, bin, rotacao))
                    {

                        if(PrintEP && PrintConst)
std::cout<<"\t\t\t"<<"Can insert (Rotacionado)\n";

                        vetIdEpRot[numEps].epId = ep;
                        vetIdEpRot[numEps].r    = rotacao;
                        vetIdEpRot[numEps].atributo = get<1>(getMinArray(bin.vetEp[ep].vetDim, instanciaG.numDim));

                        numEps += 1;
                    }
                    if(PrintEP && PrintConst)
std::cout << "\t\t\t" << "Can't insert\n";
                }
            }

            if(numEps == 0)
                continue;   // FOR(int b=0; b < tamVetBinArea; ++b)

            // Escolhe o EP com menor coordenada
            int idVetIdEp = 0;
            if(numEps > 1)
            {
                std::sort(vetIdEpRot.begin(), vetIdEpRot.begin()+numEps);
                int tam = std::max(1, int(numEps*input.aphaBinEscolhaEp));
                idVetIdEp = getRandInt(0, tam-1);
            }

            if(PrintConst)
std::cout<<"\t\tAdd item a EP"<<bin.vetEp[vetIdEpRot[idVetIdEp].epId].print()<<")\n";

            // Adicionar o item ao bin
            bin.addItem(vetIdEpRot[idVetIdEp].epId, itemId, vetIdEpRot[idVetIdEp].r);
            realizouInsercao = true;
            break;  // FOR(int b=0; b < tamVetBinArea; ++b)

        } // END FOR(int b=0; b < tamVetBinArea; ++b)

        if(!realizouInsercao)
        {
            // Tanta criar um novo bin
            int binVazioId = getBinVazio(vetBin, vetBinTam);
            if(binVazioId >= 0)
            {
                Bin &bin = vetBin[binVazioId];
                bool itemMaior = false;

                // Verifica se o item cabe no bin
                #pragma GCC unroll 3
                for(int d=0; d < 3; ++d)
                {
                    if(instanciaG.vetItens[itemId].vetDim[d] > bin.binDim[d])
                    {
                        itemMaior = true;
                        break;
                    }

                    if((d+1) == instanciaG.numDim)
                        break;
                }

                // TODO: Percorrer bins vazios
                if(itemMaior)
                    break;

                bin.addEp(PontoZero);
                bin.addItem(0, itemId, Rot0);
                numItensAlocados += 1;

                if(PrintConst)
std::cout << "\t\tAdd item ao bin(" << binVazioId << ") vazio\n\n";

            }
        }
        else
            numItensAlocados += 1;
    } // END FOR(int i=0; i < numItens; ++i)

//std::cout<<"Sequencia de itens: "<<seqItens.printN(instanciaG.numItens)<<"\n";


    if(PrintConst)
    {
std::cout << "Num de itens Alocados: " << numItensAlocados << " de um total de " << vetItensTam << "\n\n";
std::cout << "************FIM CONSTRUTIVO_BIN_PACKING************\n";
std::cout << "***************************************************\n";
    }



    return numItensAlocados;

}


/* Aplica o construtivo por <numRepeticoes> vezes ate encontrar uma solucao
      com todos os itens alocados.
*/
bool ConstrutivoBinNS::construtivoBinPacking(SolucaoNS::Bin &bin,
                                             VectorI &vetItens,
                                             const int vetItensTam,
                                             const double alpha,
                                             const int numRepeticoes)
{
    double volume  = bin.volumeOcupado;
    double demanda = bin.demandaTotal;

    for(int i=0; i < vetItensTam; ++i)
    {
        volume  += instanciaG.vetItens[vetItens[i]].volume;
        demanda += instanciaG.vetItens[vetItens[i]].peso;
    }

    if(volume > bin.volumeTotal || demanda > instanciaG.veicCap)
        return false;

    //std::cout<<"construtivoBinPacking\n";

    static Vector<SolucaoNS::Bin> binVet(1);
    static VectorI vetItensAux(instanciaG.numItens);

    for(int i=0; i < numRepeticoes; ++i)
    {
        copiaBin(bin, binVet[0]);
        copiaVet(vetItens, vetItensAux, vetItensTam);

        //std::cout<<"vetItensAux: "<<vetItensAux<<"\n\n";

        int numItensAlo = construtivoBinPacking(binVet,
                                                1,
                                                vetItensAux,
                                                vetItensTam,
                                                alpha);

        //std::cout<<"numItensAlo: "<<numItensAlo<<"\n\n";
        if(numItensAlo == vetItensTam)
        {
            copiaBin(binVet[0], bin);
            return true;
        }
    }

    /*
    if(input.cpSat)
    {
        copiaVet(vetItens, vetItensAux, vetItensTam);

        for(int i=0; i < bin.numItens; ++i)
            vetItensAux[i+vetItensTam] = binVet[0].vetItemId[i];

        if(cpSatBinPacking(binVet[0], vetItensAux, bin.numItens+vetItensTam))
        {
            std::cout<<"CP-SAT Encontrou Solucao Viavel!\n";
            copiaBin(binVet[0], bin);
            return true;
        }

        return false;
    }
    */

    return false;
}

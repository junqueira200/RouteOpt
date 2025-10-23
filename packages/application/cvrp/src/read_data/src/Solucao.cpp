/* ****************************************
 * ****************************************
 *  Data:    05/11/24
 *  Arquivo: Solucao.cpp
 *  Autor:   Igor de Andrade Junqueira
 *  Projeto: 2L-SDVRP
 * ****************************************
 * ****************************************/

#include <format>
#include "Solucao.h"
#include "sefe_array.h"
#include "ConstrutivoBin.h"

using namespace InstanciaNS;
using namespace ConstrutivoBinNS;


SolucaoNS::Bin::Bin()
{

    vetPosItem = Vector<Ponto>(NumItensPorBin);
    vetEp      = Vector<Ponto>(NumEpPorBin);
    vetItemId  = VectorI(NumItensPorBin);
    vetPosItem.setAll(Ponto());
    vetRotacao = Vector<Rotacao>(NumItensPorBin);
    vetRotacao.setAll(Rot0);

    vetItens   = Vector<int8_t>(instanciaG.numItens);
    vetItens.setAll((int8_t)0);


    #pragma GCC uroll 3
    for(int d=0; d < 3; ++d)
    {
        binDim[d] = instanciaG.vetDimVeiculo[d];
    }


    volumeTotal = 1;

    #pragma GCC uroll 3
    for(int d=0; d < 3; ++d)
    {
        volumeTotal *= binDim[d];

        if((d+1) == instanciaG.numDim)
            break;
    }
}

std::string SolucaoNS::Ponto::print() const
{
    std::string str = "(";
    for(int i=0; i < instanciaG.numDim; ++i)
    {
        str += std::format("{:.1f}", vetDim[i]);
        if(i < instanciaG.numDim-1)
            str += ",";
    }

    str += ")";
    return str;
}

// Add item na posicao x,y e cria dois EPs TODO: verificar!
void SolucaoNS::Bin::addItem(int idEp, int idItem, InstanciaNS::Rotacao r)
{

    if(vetPosItem.size() < (numItens+1))
    {
        vetPosItem.push_back(Ponto());
        vetItemId.push_back(0);
        vetRotacao.push_back(Rot0);

    }


    // Add item
    vetItemId[numItens] = idItem;
    vetPosItem[numItens].vetDim = vetEp[idEp].vetDim;
    vetRotacao[numItens] = r;
    vetItens[idItem] = (int8_t)1;

    numItens      += 1;
    volumeOcupado += instanciaG.vetItens[idItem].volume;
    demandaTotal  += instanciaG.vetItens[idItem].peso;


    // Cria dois novos EPs
    Ponto ponto;
    ponto = vetPosItem[numItens-1];
    ponto.vetDim[0] += instanciaG.vetItens[idItem].getDimRotacionada(0, r);

    if(PrintEP)
std::cout<<"posItem: "<<vetPosItem[numItens-1].print()<<"\n";

    if(PrintEP)
std::cout<<"\t\t\tCriando EP"<<ponto.print()<<"\n";

    addEp(ponto);


    ponto = vetPosItem[numItens-1];
    ponto.vetDim[1] += instanciaG.vetItens[idItem].getDimRotacionada(1, r);

    if(PrintEP)
std::cout<<"\t\t\tCriando EP"<<ponto.print()<<"\n";
    addEp(ponto);
    rmI_Ep(idEp);

    // Percorre os EPs antigos e verifica colisao com o novo item
    static VectorI vetEpRm(NumEpPorBin);
    int tamVetEpRm = 0;

    if(vetEpRm.size() < numEps)
        vetEpRm.resize(numEps);

    for(int i=0; i < numEps-2; ++i)
    {
        if(epColideItem(vetEp[i], vetPosItem[numItens-1], idItem))
        {
            vetEpRm[tamVetEpRm] = i;
            tamVetEpRm += 1;
        }
    }

    for(int i=tamVetEpRm-1; i >= 0; --i)
        rmI_Ep(vetEpRm[i]);

}

int SolucaoNS::Bin::getEpComMenorCoord(const VectorI &vetIdEp, int tam)
{

//std::cout<<"\t\t\tvetIdEp: "<<vetIdEp.printN(tam)<<"\n";
//std::cout<<"\t\t\tvetEp: ";


    int menorId = 0;
    double dimMin  = get<1>(getMinArray(vetEp[vetIdEp[menorId]].vetDim, instanciaG.numDim));
//std::cout<<vetEp[vetIdEp[menorId]].print()<<" ["<<dimMin<<"]; ";

    // Percorre o vetor de indices
    for(int i=1; i < tam; ++i)
    {
        int id = vetIdEp[i];

//std::cout<<vetEp[id].print()<<" ";

        double dimMinTemp = get<1>(getMinArray(vetEp[id].vetDim, instanciaG.numDim));
//std::cout<<"["<<dimMinTemp<<"]; ";
        if(dimMinTemp < dimMin)
        {
            menorId = i;
            dimMin = dimMinTemp;
        }
    }

//std::cout<<"\n\t\t\tMenor dim: "<<dimMin<<"\n\n";

    return menorId;
}

void SolucaoNS::Bin::reset()
{
    numItens      = 0;
    numEps        = 0;
    volumeOcupado = 0.0;
    demandaTotal  = 0.0;
    vetItens.setAll((int8_t)0);
}

bool SolucaoNS::Bin::rmI_Item(int i)
{
    if(i >= numItens)
        return false;

    int itemId = vetItemId[i];
    vetItens[itemId] = (int8_t)0;
    demandaTotal -= instanciaG.vetItens[itemId].peso;

    if(numItens == 1)
    {
        numItens = 0;
        return true;
    }

    // Shifit vetPosItem, vetItemId para a direita

    for(int ii=i; ii < (numItens-1); ++ii)
        vetPosItem[ii] = vetPosItem[ii+1];

    for(int ii=i; ii < (numItens-1); ++ii)
        vetItemId[ii] = vetItemId[ii+1];

    for(int ii=i; ii < (numItens-1); ++ii)
        vetRotacao[ii] = vetRotacao[ii+1];

    numItens -= 1;

    return true;
}

void SolucaoNS::Bin::addEp(const Ponto &ep)
{
    if(PrintEP)
std::cout<<"addEp"<<ep.print()<<"\n";

    if(vetEp.size() < (numEps+1))
        vetEp.push_back(Ponto());

    #pragma GCC unroll 3
    for(int d=0; d < 3; ++d)
    {
        if(PrintEP)
std::cout<<"\t\t\t\t("<<ep.vetDim[d]<<"; "<<binDim[d]<<")\n";
        if(ep.vetDim[d] >= binDim[d])
            return;

        if((d+1) == instanciaG.numDim)
            break;
    }

    if(PrintEP)
std::cout<<"\t\t\t\t"<<"EP criado\n\n";
    //std::cout<<"Cria EP("<<x<<","<<y<<")\n\n";

    vetEp[numEps] = ep;
    numEps += 1;
}

bool SolucaoNS::Bin::rmI_Ep(int i)
{

    if(i >= numEps)
        return false;

    if(numEps == 1)
    {
        numEps = 0;
        return true;
    }

    // Shifit vetEpX, vetEpY para a esquerda

    for(int ii=i; ii < (numEps-1); ++ii)
        vetEp[ii] = vetEp[ii+1];

    numEps -= 1;
    return true;
}

SolucaoNS::Solucao::Solucao(const InstanciaNS::Instancia &instancia)
{
    vetBin      = Vector<Bin>(instancia.numVeiculos);
    for(int b=0; b < instancia.numVeiculos; ++b)
    {
        #pragma GCC uroll 3
        for(int d=0; d < 3; ++d)
        {
            vetBin[b].binDim[d] = instancia.vetDimVeiculo[d];
        }

        vetBin[b].volumeTotal = 1;

        #pragma GCC uroll 3
        for(int d=0; d < 3; ++d)
        {
            vetBin[b].volumeTotal *= vetBin[b].binDim[d];

            if((d+1) == instancia.numDim)
                break;
        }

    }

    vetRota       = Vector<Rota>(instancia.numVeiculos);

    for(int r=0; r < instancia.numVeiculos; ++r)
        vetRota[r].binPtr = &vetBin[r];

//    vetItemRotaId = VectorI(instancia.numItens);
//    vetItemRotaId.setAll(-1);

}

bool SolucaoNS::Solucao::verificaSol(std::string &error)
{

    static VectorD vetDem(instanciaG.numClientes);
    static VectorI vetClieAtend(instanciaG.numClientes);
    static VectorI vetItens(instanciaG.numItens);

    vetDem.setAll(0.0);
    vetClieAtend.setAll(0);
    vetItens.setAll(0);

    for(int r=0; r < instanciaG.numVeiculos; ++r)
    {
        Rota &rota = vetRota[r];
        double demTotal = 0.0;

        for(int i=1; i < (rota.numPos-1); ++i)
        {
            double temp = rota.vetDemClie[rota.vetRota[i]];
            vetDem[rota.vetRota[i]] += temp;
            demTotal += temp;
            vetClieAtend[rota.vetRota[i]] += 1;
        }

        if(!doubleEqual(demTotal, rota.binPtr->demandaTotal, 1E-5))
        {
            error += "Demandas sao diferentes, rota("+std::format("{}", rota.binPtr->demandaTotal)+
                     "); calculado("+std::format("{}", demTotal)+"\n";

            return false;
        }
    }

    // Verifica se todos os clientes sao visitados
    int num = 1;
    if(instanciaG.split)
        num = std::numeric_limits<int>::infinity();

    for(int i=1; i < instanciaG.numClientes; ++i)
    {
        if(!(vetClieAtend[i] > 0 && vetClieAtend[i] <= num))
        {
            if(vetClieAtend[i] == 0)
                error += "Cliente("+std::to_string(i)+") nao atendido\n";
            else if(!instanciaG.split && vetClieAtend[i] > 1)
                error += "Cliente("+std::to_string(i)+") atendido mais de uma vez\n";

            return false;
        }

        if(!doubleEqual(vetDem[i], instanciaG.vetDemandaCliente[i]))
        {
            error += "Cliente("+std::to_string(i)+") nao recebeu sua demanda completa\n";
            return false;
        }
    }

    // Verificar os bins
    for(int b=0; b < instanciaG.numVeiculos; ++b)
    {
        Bin &bin = vetBin[b];

        for(int i=0; i < bin.numItens; ++i)
            vetItens[bin.vetItemId[i]] += 1;

        if(!bin.verificaViabilidade())
        {
            error += "Colisao entre itens no bin("+std::to_string(b)+")\n";
            return false;
        }
    }

    for(int i=0; i < instanciaG.numItens; ++i)
    {
        if(vetItens[i] != 1)
        {
            error += "Item eh entregue ("+std::to_string(vetItens[i]) + ") vezes\n";
            return false;
        }
    }

    double distTotalTemp = 0.0;

    for(int r=0; r < instanciaG.numVeiculos; ++r)
    {
        double distRota = 0.0;
        Rota &rota = vetRota[r];

        for(int i=0; i < (rota.numPos-1); ++i)
        {
            int cli0 = rota.vetRota[i];
            int cli1 = rota.vetRota[i+1];

            distRota += instanciaG.matDist(cli0, cli1);
        }

        //std::cout<<"Dist Rota: "<<distRota<<"\n";

        if(!doubleEqual(distRota, rota.distTotal, DiffPermetidaDist))
        {
            error += "Rota: "+rota.printRota()+"; esta com a distancia errada. Dist calculada("+std::to_string(distRota)+")";
            return false;
        }

        distTotalTemp += distRota;
    }

    if(!doubleEqual(distTotalTemp, distTotal, DiffPermetidaDist))
    {
        error += "Dist total esta errada, calculada("+std::to_string(distTotalTemp)+"), dist sol("+std::to_string(distTotal)+")";
        return false;
    }



    return true;

}

int SolucaoNS::Solucao::getBinVazio()
{
    for(int b=0; b < vetRota.size(); ++b)
    {
        if(vetBin[b].vazio())
            return b;
    }

    return -1;
}

void SolucaoNS::Solucao::copiaSolucao(const SolucaoNS::Solucao &sol)
{

    distTotal = sol.distTotal;
    for(int b=0; b < instanciaG.numVeiculos; ++b)
    {
        copiaBin(sol.vetBin[b], vetBin[b]);
        copiaRota(sol.vetRota[b], vetRota[b]);
    }
}

void SolucaoNS::Solucao::reset()
{

    distTotal = 0.0;
    for(int r=0; r < instanciaG.numVeiculos; ++r)
    {
        vetRota[r].reset();
        vetBin[r].reset();
    }
}

double SolucaoNS::Solucao::getUtilizacaoMediaBins() const
{
   double val = 0.0;
   for(int i=0; i < instanciaG.numVeiculos; ++i)
       val += vetBin[i].getPorcentagemUtilizacao();

    return val/instanciaG.numVeiculos;
}

double SolucaoNS::Solucao::getUtilizacaoMedianaBins() const
{
    VectorD vetUtil(instanciaG.numVeiculos, 0.0);

    for(int i=0; i < instanciaG.numVeiculos; ++i)
        vetUtil[i] = vetBin[i].getPorcentagemUtilizacao();

    std::sort(vetUtil.begin(), vetUtil.end());

    //std::cout<<vetUtil<<"\n\n";

    if((instanciaG.numVeiculos%2) == 0)
        return (vetUtil[instanciaG.numVeiculos/2]+vetUtil[(instanciaG.numVeiculos/2)+1])/2.0;
    else
        return vetUtil[instanciaG.numVeiculos/2];

}

double SolucaoNS::Solucao::getTamMedianaRota() const
{
    VectorI vetTam(instanciaG.numVeiculos, 0);

    for(int i=0; i < instanciaG.numVeiculos; ++i)
        vetTam[i] = vetRota[i].numPos - 2;

    std::sort(vetTam.begin(), vetTam.end());

    if((instanciaG.numVeiculos%2) == 0)
        return ((double)vetTam[instanciaG.numVeiculos/2]+vetTam[(instanciaG.numVeiculos/2)+1])/2.0;
    else
        return vetTam[instanciaG.numVeiculos/2];

}

void SolucaoNS::copiaRota(const Rota &rotaFonte, Rota &rota)
{
    rota.numPos    = rotaFonte.numPos;
    //rota.demTotal  = rotaFonte.demTotal;
    rota.distTotal = rotaFonte.distTotal;

    copiaVet(rotaFonte.vetRota, rota.vetRota, rota.numPos);
    copiaVet(rotaFonte.vetTempoSaida, rota.vetTempoSaida, rota.numPos);
    copiaVet(rotaFonte.vetDemClie, rota.vetDemClie, rota.vetDemClie.size());
}

std::string SolucaoNS::printBinEps(const Bin &bin)
{
    std::string str;
    for(int i=0; i < bin.numEps; ++i)
    {
        str += "(";
        for(int d=0; d < instanciaG.numDim; ++d)
            str += std::to_string(bin.vetEp[i].vetDim[d]) + " ";

        str += "); ";
    }

    return std::move(str);
}


int SolucaoNS::getBinVazio(const Vector<Bin> &vetBin, int tam)
{
    int idVazio = -1;

    for(int b=0; b < tam; ++b)
    {
        if(vetBin[b].vazio())
            return b;
    }

    return -1;
}

bool SolucaoNS::verificaColisaoDoisItens(const int item0, const int item1, const Ponto &p0, const Ponto &p1, const InstanciaNS::Rotacao r0, const InstanciaNS::Rotacao r1)
{

    // Representacao dois segmentos de reta em um unica dimensao
    static Array<double,2> arrayTemp0;
    static Array<double,2> arrayTemp1;

    int numInterc = 0;

    // Verfica a intersecao em cada dimensao
    #pragma GCC unroll 3
    for(int d=0; d < 3; ++d)
    {
        arrayTemp0[0] = p0.vetDim[d];
        //arrayTemp0[1] = arrayTemp0[0] + instanciaG.vetItens[item0].vetDim[d];
        arrayTemp0[1] = arrayTemp0[0] + instanciaG.vetItens[item0].getDimRotacionada(d, r0);

        arrayTemp1[0] = p1.vetDim[d];
        arrayTemp1[1] = arrayTemp1[0] + instanciaG.vetItens[item1].getDimRotacionada(d, r1);

        double ini = std::max(arrayTemp0[0], arrayTemp1[0]);
        double fim = std::min(arrayTemp0[1], arrayTemp1[1]);

        if(ini < fim)
            numInterc += 1;
        else
            break;

        /*
        if((arrayTemp1[0] > arrayTemp0[0] && arrayTemp1[0] < arrayTemp0[1]) ||
           (arrayTemp1[1] > arrayTemp0[0] && arrayTemp1[1] < arrayTemp0[1]))
            numInterc += 1;
        else
            break;
        */

        if((d+1) == instanciaG.numDim)
            break;
    }


    return (numInterc == instanciaG.numDim);
}

bool SolucaoNS::Bin::verificaViabilidade()
{

    if(vazio())
        return true;


    for(int i=0; i < numItens; ++i)
    {
        for(int j=i+1; j < numItens; ++j)
        {
            bool a = verificaColisaoDoisItens(vetItemId[i], vetItemId[j], vetPosItem[i], vetPosItem[j], vetRotacao[i], vetRotacao[j]);
            bool b = verificaColisaoDoisItens(vetItemId[j], vetItemId[i], vetPosItem[j], vetPosItem[i], vetRotacao[j], vetRotacao[i]);

            if(a || b)
            {
                std::cout<<"EP i: "<<vetPosItem[i].print()<<"\nEP j: "<<vetPosItem[j].print()<<"\n";
                std::cout<<"Rot i: "<<vetRotacao[i]<<"\nRot j: "<<vetRotacao[j]<<"\n\n";
                std::cout<<"ItemId i: "<<vetItemId[i]<<"\nItemId j: "<<vetItemId[j]<<"\n";
                std::cout<<"Item i: "<<instanciaG.vetItens[vetItemId[i]].vetDim<<"\n";
                std::cout<<"Item j: "<<instanciaG.vetItens[vetItemId[j]].vetDim<<"\n";

                if(a != b)
                    std::cout<<"a!=b\n";

                return false;
            }
        }
    }

    return true;

}

void SolucaoNS::Bin::rmItens(const VectorI &vetItensRm, const int tam)
{
    Ponto ponto;
    for(int i=0; i < tam; ++i)
    {
        const int item = vetItensRm[i];

        // Acha o item em vetItemId
        int posItem = -1;
        for(int j=0; j < numItens; ++j)
        {
            if(vetItemId[j] == item)
            {
                posItem = j;
                break;
            }
        }

        if(posItem == -1)
        {
            std::cout<<"Item("<<item<<") nao esta no bin!\n";
            throw "ERROR";
        }

        ponto = vetPosItem[posItem];
        //rmI_Ep(posItem);
        rmI_Item(posItem);
        addEp(ponto);
        volumeOcupado -= instanciaG.vetItens[item].volume;

    }

}

double SolucaoNS::Bin::getPorcentagemUtilizacao() const
{
    return (volumeOcupado/volumeTotal)*100;
}


std::string SolucaoNS::printPonto(const Ponto &ponto, int dim)
{
    std::string str;

    for(int d=0; d < dim; ++d)
    {
        str += std::to_string(ponto.vetDim[d]) +  " ";

        if((d+1) == dim)
            break;
    }

    return std::move(str);
}

void SolucaoNS::copiaBin(const Bin &binFonte, Bin &bin)
{


    bin.numItens      = binFonte.numItens;
    bin.numEps        = binFonte.numEps;
    bin.binDim        = binFonte.binDim;
    bin.demandaTotal  = binFonte.demandaTotal;
    bin.volumeTotal   = binFonte.volumeTotal;
    bin.volumeOcupado = binFonte.volumeOcupado;

    if(binFonte.vetEp.size() < bin.vetEp.size())
        bin.vetEp.resize(binFonte.vetEp.size());

    if(binFonte.vetPosItem.size() < bin.vetPosItem.size())
    {
        bin.vetPosItem.resize(binFonte.vetPosItem.size());
        bin.vetItemId.resize(binFonte.vetItemId.size());
        bin.vetRotacao.resize(binFonte.vetRotacao.size());
    }

    copiaVet(binFonte.vetEp, bin.vetEp, bin.numEps);
    copiaVet(binFonte.vetItemId, bin.vetItemId, bin.numItens);
    copiaVet(binFonte.vetPosItem, bin.vetPosItem, bin.numItens);
    copiaVet(binFonte.vetRotacao, bin.vetRotacao, bin.numItens);
    copiaVet(binFonte.vetItens, bin.vetItens, bin.vetItens.size());

}

SolucaoNS::Rota::Rota()
{
    vetRota       = VectorI(TamRota);
    vetTempoSaida = VectorD(TamRota);
    vetDemClie    = VectorD(instanciaG.numClientes);

    vetRota.setAll(0);
    vetTempoSaida.setAll(0.0);
    vetDemClie.setAll(0.0);
}

void SolucaoNS::Rota::reset()
{
    numPos           = 2;
    distTotal        = 0.0;
    //demTotal         = 0;
    vetRota[0]       = 0;
    vetRota[1]       = 0;
    vetDemClie.setAll(0);
    vetTempoSaida[0] = 0;
    vetTempoSaida[1] = 0;
}

std::string SolucaoNS::Rota::printRota()
{
    std::string str;
    str += "Dist("+std::to_string(distTotal) + "); ";

    for(int i=0; i < numPos; ++i)
        str += std::to_string(vetRota[i])+ " ";

    return str;
}

std::ostream& SolucaoNS::operator<<(std::ostream &os, const Solucao &sol)
{
    os<<"DISTANCIA TOTAL: "<<sol.distTotal<<"\n\n";

    for(int r=0; r < instanciaG.numVeiculos; ++r)
    {
        os<<"ROTA("<<r<<"); DIST("<<sol.vetRota[r].distTotal<<"); DEM("<<sol.vetRota[r].binPtr->demandaTotal<<"); ";
        for(int i=0; i < sol.vetRota[r].numPos; ++i)
            os<<sol.vetRota[r].vetRota[i]<<" ";

        os<<"\n\n";
    }

    os<<"BIN:\n";
    for(int r=0; r < instanciaG.numVeiculos; ++r)
    {
        os<<sol.vetBin[r]<<"\n";
    }

    return os;
}

std::ostream& SolucaoNS::operator<<(std::ostream &os, const Bin& bin)
{
    os<<"ITENS: ";
    for(int i=0; i < bin.numItens; ++i)
        os<<bin.vetItemId[i]<<" ";
    os<<"\n";

    os<<"Posicoes: ";
    for(int i=0; i < bin.numItens; ++i)
    {
        os<<bin.vetPosItem[i].print()<<" ";
    }


    return os;
}

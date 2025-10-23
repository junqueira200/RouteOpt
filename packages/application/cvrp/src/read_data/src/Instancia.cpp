/* ****************************************
 * ****************************************
 *  Data:    05/11/24
 *  Arquivo: Instancia.cpp
 * ****************************************
 * ****************************************/

#include "Instancia.h"
#include "AuxT.h"
#include <cuchar>
#include <fstream>
#include "InputOutput.h"

using namespace InstanciaNS;
using namespace ParseInputNS;

InstanciaNS::Instancia::Instancia(int numClientes_, int numItens_, int numVeiculos_):
                                                                   matDist(numClientes_, numClientes_),
                                                                   matTempo(numClientes, numClientes),
                                                                   //vetItens(numItens_),
                                                                   vetPesoItens(numItens_),
                                                                   //vetItemLargura(numItens_),
                                                                   vetDemandaCliente(numClientes_),
                                                                   vetItemCliente(numItens_),
                                                                   matCliItensIniFim(0, 0),
                                                                   vetNumItensPorCli(numClientes_),
                                                                   vetMinDimItens(numItens_),
                                                                   vetTw(numClientes_)

{

    numClientes = numClientes_;
    numItens    = numItens_;
    numVeiculos = numVeiculos_;

    if(numClientes < 0 || numItens < (numClientes-1))
    {
        assertm(true, "numClientes("<<numClientes<<") < 0 || numItens("<<numItens<<") < numClientes-1("<<numClientes<<")");
    }

    matCliItensIniFim = Matrix<int>(numClientes, 2);
    matCliItensIniFim.setVal(-1);

}

InstanciaNS::Instancia::Instancia():
                                    matDist(0, 0),
                                    matTempo(0,0),
                                    vetItens(0),
                                    vetDemandaCliente(0),
                                    vetItemCliente(0, 0),
                                    matCliItensIniFim(0, 0),
                                    vetPesoItens(0)

{

}


void InstanciaNS::leInstancia(const std::string &strFile)
{

    instanciaG.matTempo.setVal(1.0);

    int numClientes, numVeiculos, numItens;

    instanciaG.nome = strFile;
    std::ifstream file(strFile);
    assertm(!file.is_open(), "Nao foi possivel abrir o arquivo: "<<strFile);

    //std::cout<<"File("<<strFile<<") aberto\n";

    std::string lineLixo;
    int num = 2;
    if(input.splitInstancia)
        num = 1;
    for(int i=0; i < num; ++i)
    {
        getline(file, lineLixo);
        //std::cout << "line: " << lineLixo << "\n";
    }

    file>>numClientes;
    numClientes += 1;

    for(int i=0; i < 1; ++i)
    {
        getline(file, lineLixo);
        //std::cout << "line: " << lineLixo << "\n";
    }

    file>>numVeiculos;

    for(int i=0; i < 1; ++i)
    {
        getline(file, lineLixo);
        //std::cout << "line: " << lineLixo << "\n";
    }

    file>>numItens;
    //numItens += 1;

//std::cout<<"numClientes: "<<numClientes<<"; numItens: "<<numItens<<" numVeic: "<<numVeiculos<<"\n";


    if(!input.splitInstancia)
    {
        for(int i = 0; i < 2; ++i)
        {
            getline(file, lineLixo);
            //std::cout << "line: " << lineLixo << "\n";
        }
    }

    instanciaG = Instancia(numClientes, numItens, numVeiculos);
    file >> instanciaG.veicCap >> instanciaG.vetDimVeiculo[0] >> instanciaG.vetDimVeiculo[1];

//std::cout << "veicCap: " << instanciaG.veicCap << "; veicComprimento: " << instanciaG.vetDimVeiculo[0] << "; veicLargura: " << instanciaG.vetDimVeiculo[1] << "\n\n";

    num = 2;
    if(input.splitInstancia)
        num = 1;

    for(int i=0; i < num; ++i)
    {
        getline(file, lineLixo);
        //std::cout << "line: " << lineLixo << "\n";
    }

    Matrix<double> matPontos(numClientes, 2);
    matPontos.setVal(-1.0);

    for(int i=0; i < numClientes; ++i)
    {
        double x, y, cli;
        file >> cli >> x >> y >> instanciaG.vetDemandaCliente[i];
        matPontos.get(i, 0) = x;
        matPontos.get(i, 1) = y;

//std::cout<<cli<<" "<<x<<" "<<y<<" "<<instanciaG.vetDemandaCliente[i]<<"\n";

    }

    for(int i=0; i < numClientes; ++i)
    {
        instanciaG.matDist.get(i, i) = 0.0;

        for(int j=i+1; j < numClientes; ++j)
        {
            double dist = std::sqrt(std::pow(matPontos(i, 0) - matPontos(j, 0), 2) +
                                      std::pow(matPontos(i, 1) - matPontos(j, 1), 2));

            instanciaG.matDist.get(i, j) = dist;
            instanciaG.matDist.get(j, i) = dist;
        }
    }

    //std::cout << instanciaG.matDist << "\n";

    num = 2;
    if(input.splitInstancia)
        num = 1;
    for(int i=0; i < num; ++i)
    {
        getline(file, lineLixo);
        //std::cout << "line: " << lineLixo << "\n";
    }

    Vector<Vector<double>> vetVetItens(numClientes);
    int maxNumItensPorCli = 0;
    int nextItem = 0;


    //instanciaG.vetItemAltura.reserve(instanciaG.numItens);
    //instanciaG.vetItemLargura.reserve(instanciaG.numItens);
    //instanciaG.vetItemArea.reserve(instanciaG.numItens);

    instanciaG.vetPesoItens.reserve(instanciaG.numItens);
    instanciaG.vetItens.reserve(instanciaG.numItens);

    num = numClientes;
    if(input.splitInstancia)
        num -= 1;
    for(int i=0; i < num; ++i)
    {
        int node, numItensPorClie;

        file >> node >> numItensPorClie;
//std::cout<<node<<" "<<numItensPorClie<<"\n";

        maxNumItensPorCli = std::max(maxNumItensPorCli, numItensPorClie);

        if(node != 0)
        {
            instanciaG.matCliItensIniFim.get(node, 0) = nextItem;
            //instancia.matCliItensIniFim.get(node, 1) = nextItem+(numItensPorClie-1);
        }

        for(int item = 0; item < numItensPorClie; ++item)
        {
            double altura, comprimento;
            file >> altura >> comprimento;

            if(input.comprimentoAlturaIguais1)
            {
                altura      = 1.0;
                comprimento = 1.0;
            }

            vetVetItens[i].push_back(altura);
            vetVetItens[i].push_back(comprimento);
            instanciaG.vetItemCliente[nextItem] = node;

            instanciaG.matCliItensIniFim.get(node, 1) = nextItem;

            nextItem += 1;
        }

        //std::cout << vetVetItens[i] << "\n";

        for(int j = 0; j < numItensPorClie*2; j+=2)
        {
            /*
            instanciaG.vetItemAltura.push_back(vetVetItens[i][j]);
            instanciaG.vetItemLargura.push_back(vetVetItens[i][j + 1]);
            instanciaG.vetItemArea.push_back(vetVetItens[i][j] * vetVetItens[i][j + 1]);
            */

            instanciaG.vetItens.push_back(Item(vetVetItens[i][j], vetVetItens[i][j+1], 0.0, instanciaG.vetDemandaCliente[node]/numItensPorClie));

        }
    }

    instanciaG.maxNumItensPorClie = maxNumItensPorCli;
    instanciaG.atualizaVetMinDimItens();

    file.close();

}

void InstanciaNS::Instancia::atualizaVetMinDimItens()
{
    for(int i=0; i < numItens; ++i)
    {
        auto tuple = getMinArray(vetItens[i].vetDim, numDim);
        vetMinDimItens[i] = get<1>(tuple);
    }

    //std::cout<<"vetMinDimItens: "<<vetMinDimItens<<"\n\n";
}

std::string InstanciaNS::Item::print(bool printVol)
{
    std::string str = "(";
    for(int i=0; i < instanciaG.numDim; ++i)
    {
        str += std::to_string(vetDim[i]);
        if(i < (instanciaG.numDim-1))
            str += ",";
    }

    if(printVol)
        str += "; " + std::to_string(volume);

    str += "; " + std::format("{:.1f}", peso);

    str += ")";
    return str;
}


/*std::string InstanciaNS::printItem(int itemId)
{
    return std::to_string(instanciaG.vetItemLargura[itemId])+","+std::to_string(instanciaG.vetItemAltura[itemId]);
}*/

InstanciaNS::Item::Item(double x, double y, double z, double peso_)
{
    vetDim[0] = x;
    vetDim[1] = y;
    vetDim[2] = z;
    peso = peso_;
    volume = 1.0;

    for(int d=0; d < 3; ++d)
    {
        volume *= vetDim[d];
        if((d+1) == instanciaG.numDim)
            break;
    }
}

void InstanciaNS::Item::set(double x, double y, double z)
{
    vetDim[0] = x;
    vetDim[1] = y;
    vetDim[2] = z;

    volume = 1.0;

    for(int d=0; d < 3; ++d)
    {
        volume *= vetDim[d];
        if((d+1) == instanciaG.numDim)
            break;
    }
}

double InstanciaNS::Item::getDimRotacionada(int d, Rotacao r)
{
    if(r == Rot0)
        return vetDim[d];
    else if(r == Rot1)
        return vetDim[(d+1)%2];
    else
    {
        PRINT_DEBUGG("", "ERRO, Rotacao("<<r<<") Nao eh implementado!");
        throw "ERRO";
    }
}

int InstanciaNS::copiaItensCliente(int cliente, VectorI &vetItens)
{
    int numItens = 0;

    for(int i=instanciaG.matCliItensIniFim(cliente, 0); i <= instanciaG.matCliItensIniFim(cliente, 1); ++i)
    {
        vetItens[numItens] = i;
        numItens += 1;
    }

    return numItens;
}


int InstanciaNS::copiaItensClientes(VectorI& vetClientes, int tam, VectorI& vetItens)
{
    int tamVetItens = 0;
    for(int i=0; i < tam; ++i)
    {
        int cliente = vetClientes[i];
        const int ini = instanciaG.matCliItensIniFim(cliente, 0);
        const int fim = instanciaG.matCliItensIniFim(cliente, 1);

        for(int j=ini; j <= fim; ++j)
        {
            vetItens[tamVetItens] = j;
            tamVetItens += 1;
        }
    }

    return tamVetItens;
}


double InstanciaNS::calculaDistancia(VectorI& vet, int tam)
{
    double dist = 0.0;
    if(tam <= 1)
        return 0.0;

    for(int i=0; i < (tam-1); ++i)
        dist += instanciaG.matDist(vet[i], vet[i+1]);

    return dist;
}

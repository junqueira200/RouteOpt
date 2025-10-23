/* ****************************************
 * ****************************************
 *  Data:    10/02/25
 *  Arquivo: BinPackingCP.cpp
 *  Autor:   Igor de Andrade Junqueira
 *  Projeto: 2L-SDVRP
 * ****************************************
 * ****************************************/

#include "BinPackingCP.h"
#include "Instancia.h"
#include "rand.h"
#include "ConstrutivoBin.h"
#include "InputOutput.h"


using namespace SolucaoNS;
using namespace InstanciaNS;
using namespace RandNs;
using namespace BinPackingCP_NS;
using namespace ConstrutivoBinNS;
using namespace ParseInputNS;

bool BinPackingCP_NS::cpSatBinPacking(SolucaoNS::Bin &binResult, VectorI &vetItens, int tamVet)
{
    if(!input.cpSat)
    {   std::cout<<"cp-sat igual a false\n";
        return false;
    }

    binResult.reset();

    CpModelBuilder model;
    SatParameters params;
    params.set_num_workers(1);
    params.set_stop_after_first_solution(true);
    //params.set_max_time_in_seconds(5);

    if(input.cpSatTime > 0.0)
        params.set_max_time_in_seconds(input.cpSatTime);

    static Vector<IntVar> vetX(instanciaG.numItens);
    static Vector<IntVar> vetY(instanciaG.numItens);

    static Vector<IntVar> vetDx(instanciaG.numItens);
    static Vector<IntVar> vetDy(instanciaG.numItens);

    static Vector<IntVar> vetR(instanciaG.numItens);

    /*
    Domain domainX(0, ((int)instanciaG.vetDimVeiculo[0]));
    Domain domainY(0, ((int)instanciaG.vetDimVeiculo[1]));
    Domain domainZ(0, ((int) instanciaG.vetDimVeiculo[0]*instanciaG.vetDimVeiculo[1]));

    IntVar X = model.NewIntVar(domainX).WithName("X");
    IntVar Y = model.NewIntVar(domainX).WithName("Y");
    IntVar Z = model.NewIntVar(domainZ).WithName("Z");

    model.Minimize(Z);

    model.AddMultiplicationEquality(Z, {X,Y});
    */


    for(int i=0; i < tamVet; ++i)
    {
        const Item item = instanciaG.vetItens[vetItens[i]];
        Domain domainX(0, ((int)instanciaG.vetDimVeiculo[0]));
        Domain domainY(0, ((int)instanciaG.vetDimVeiculo[1]));

        IntVar x = model.NewIntVar(domainX).WithName("x_" + std::to_string(i));
        vetX[i] = x;


        IntVar y = model.NewIntVar(domainY).WithName("y_" + std::to_string(i));
        vetY[i] = y;

        int max = (int)std::max(item.vetDim[0], item.vetDim[1]);
        int min = (int)std::min(item.vetDim[0], item.vetDim[1]);

        Domain domainDxDy(min, max);

        IntVar dx = model.NewIntVar(domainDxDy).WithName("dx_"+std::to_string(i));
        vetDx[i] = dx;


        IntVar dy = model.NewIntVar(domainDxDy).WithName("dy_"+std::to_string(i));
        vetDy[i] = dy;

        Domain domainRot(0, 1);
        IntVar r = model.NewIntVar(domainRot).WithName("r_"+std::to_string(i));
        vetR[i] = r;

        model.AddLessOrEqual(x+dx, (int)instanciaG.vetDimVeiculo[0]);
        model.AddLessOrEqual(y+dy, (int)instanciaG.vetDimVeiculo[1]);

        model.AddElement(r, {(int)item.vetDim[0], (int)item.vetDim[1]}, dx);
        model.AddElement(r, {(int)item.vetDim[1], (int)item.vetDim[0]}, dy);

        if((int)item.vetDim[0] == (int)item.vetDim[1])
            model.AddEquality(r, 0);

    }

    for(int i=0; i < tamVet; ++i)
    {
        for(int j=i+1; j < tamVet; ++j)
        {
            BoolVar no_overlap_x1 = model.NewBoolVar();
            BoolVar no_overlap_x2 = model.NewBoolVar();
            BoolVar no_overlap_y1 = model.NewBoolVar();
            BoolVar no_overlap_y2 = model.NewBoolVar();

            model.AddLessOrEqual(vetX[i] + vetDx[i], vetX[j]).OnlyEnforceIf(no_overlap_x1);
            model.AddLessOrEqual(vetX[j] + vetDx[j], vetX[i]).OnlyEnforceIf(no_overlap_x2);


            model.AddLessOrEqual(vetY[i] + vetDy[i], vetY[j]).OnlyEnforceIf(no_overlap_y1);
            model.AddLessOrEqual(vetY[j] + vetDy[j], vetY[i]).OnlyEnforceIf(no_overlap_y2);

            model.AddBoolOr({no_overlap_x1, no_overlap_x2, no_overlap_y1, no_overlap_y2});
        }
    }

    const CpSolverResponse response = SolveWithParameters(model.Build(), params);

    if(response.status() == FEASIBLE || response.status() == OPTIMAL)
    {
        for(int i = 0; i < tamVet; ++i)
        {
            Item item = instanciaG.vetItens[vetItens[i]];

            int x  = SolutionIntegerValue(response, vetX[i]);
            int y  = SolutionIntegerValue(response, vetY[i]);
            int r  = SolutionIntegerValue(response, vetR[i]);

            int dx = SolutionIntegerValue(response, vetDx[i]);
            int dy = SolutionIntegerValue(response, vetDy[i]);

            //bin.addItem(vetItens[i], )
            binResult.vetItemId[i] = vetItens[i];
            binResult.vetItens[vetItens[i]] = 1;
            binResult.vetPosItem[i] = Ponto(x, y, 0);
            binResult.vetRotacao[i] = Rotacao(r);
            binResult.volumeOcupado += instanciaG.vetItens[vetItens[i]].volume;
            binResult.demandaTotal  += instanciaG.vetItens[vetItens[i]].peso;
            binResult.numItens += 1;

            //std::cout<<x<<" "<<y<<"; "<<dx<<" "<<dy<<"\n";
            //std::cout<<instanciaG.vetItens[vetItens[i]].vetDim<<"\n\n";
        }

        if(!binResult.verificaViabilidade())
        {

            std::cout<<"CP-SAT Encontrou Solucao Viavel!\n";
            std::cout << "ERROR, Bin NAO eh Viavel!\n";
            PRINT_DEBUGG("", "");
            exit(-1);
        }

        criaEPs(binResult);
        //std::cout<<"CORRETO!\n";
        return true;
    }
    //std::cout<<"INCORRETO!\n";
    return false;

}

bool BinPackingCP_NS::cpSatBinPacking2(SolucaoNS::Bin &binResult, VectorI &vetItens, int tamVet)
{
    if(!input.cpSat)
    {   std::cout<<"cp-sat igual a false\n";
        return false;
    }

    binResult.reset();

    CpModelBuilder model;
    SatParameters params;
    params.set_num_workers(4);
    params.set_stop_after_first_solution(true);
    params.set_max_time_in_seconds(10);

    if(input.cpSatTime > 0.0)
        params.set_max_time_in_seconds(input.cpSatTime);

    static VectorI         vetItens2(instanciaG.numItens*2);
    static Vector<IntVar>  vetX(instanciaG.numItens*2);
    static Vector<IntVar>  vetY(instanciaG.numItens*2);
    static Vector<BoolVar> vetUsado(instanciaG.numItens*2);
    static Vector<int>     vetDx(instanciaG.numItens*2);
    static Vector<int>     vetDy(instanciaG.numItens*2);

    for(int i=0; i < tamVet; ++i)
    {
        vetItens2[i]        = vetItens[i];
        vetItens2[i+tamVet] = vetItens[i];

        Item& item = instanciaG.vetItens[vetItens[i]];

        vetDx[i] = item.vetDim[0];
        vetDy[i] = item.vetDim[1];

        vetDx[i+tamVet] = item.vetDim[1];
        vetDy[i+tamVet] = item.vetDim[0];
    }

    int tamVet2 = 2*tamVet;

    //static Vector<IntVar> vetR(instanciaG.numItens);

    /*
    Domain domainX(0, ((int)instanciaG.vetDimVeiculo[0]));
    Domain domainY(0, ((int)instanciaG.vetDimVeiculo[1]));
    Domain domainZ(0, ((int) instanciaG.vetDimVeiculo[0]*instanciaG.vetDimVeiculo[1]));

    IntVar X = model.NewIntVar(domainX).WithName("X");
    IntVar Y = model.NewIntVar(domainX).WithName("Y");
    IntVar Z = model.NewIntVar(domainZ).WithName("Z");

    model.Minimize(Z);

    model.AddMultiplicationEquality(Z, {X,Y});
    */


    for(int i=0; i < tamVet2; ++i)
    {
        Domain domainX(0, ((int)instanciaG.vetDimVeiculo[0]));
        Domain domainY(0, ((int)instanciaG.vetDimVeiculo[1]));

        IntVar x = model.NewIntVar(domainX).WithName("x0_" + std::to_string(i));
        IntVar y = model.NewIntVar(domainY).WithName("y0_" + std::to_string(i));

        vetX[i] = x;
        vetY[i] = y;


        Domain domainRot(0, 1);
        BoolVar r = model.NewBoolVar().WithName("r_"+std::to_string(i));
        vetUsado[i] = r;


        model.AddLessOrEqual(vetX[i]+vetDx[i], (int)instanciaG.vetDimVeiculo[0]).OnlyEnforceIf(vetUsado[i]);
        model.AddLessOrEqual(vetY[i]+vetDy[i], (int)instanciaG.vetDimVeiculo[1]).OnlyEnforceIf(vetUsado[i]);

        if(i > tamVet)
            model.AddEquality(vetUsado[i] + vetUsado[i-tamVet], 1);
    }

    for(int i=0; i < tamVet2; ++i)
    {
        for(int j=i+1; j < tamVet2; ++j)
        {
            if(j == (i+tamVet) || i == (j+tamVet))
                continue;

            BoolVar no_overlap_x1 = model.NewBoolVar();
            BoolVar no_overlap_x2 = model.NewBoolVar();
            BoolVar no_overlap_y1 = model.NewBoolVar();
            BoolVar no_overlap_y2 = model.NewBoolVar();
            BoolVar and_ = model.NewBoolVar();

            model.AddMultiplicationEquality(and_, vetUsado[i], vetUsado[j]);


            model.AddLessOrEqual(vetX[i] + vetDx[i], vetX[j]).OnlyEnforceIf(no_overlap_x1);
            model.AddLessOrEqual(vetX[j] + vetDx[j], vetX[i]).OnlyEnforceIf(no_overlap_x2);
            model.AddLessOrEqual(vetY[i] + vetDy[i], vetY[j]).OnlyEnforceIf(no_overlap_y1);
            model.AddLessOrEqual(vetY[j] + vetDy[j], vetY[i]).OnlyEnforceIf(no_overlap_y2);
            model.AddBoolOr({no_overlap_x1, no_overlap_x2, no_overlap_y1, no_overlap_y2}).OnlyEnforceIf(and_);
        }
    }

    const CpSolverResponse response = SolveWithParameters(model.Build(), params);

    if(response.status() == FEASIBLE || response.status() == OPTIMAL)
    {
        int numItens = 0;
        for(int i = 0; i < tamVet2; ++i)
        {

            Item item = instanciaG.vetItens[vetItens2[i]];

            int x       = SolutionIntegerValue(response, vetX[i]);
            int y       = SolutionIntegerValue(response, vetY[i]);
            bool usado  = SolutionBooleanValue(response, vetUsado[i]);

            if(!usado)
                continue;

            //bin.addItem(vetItens[i], )
            binResult.vetItemId[numItens] = vetItens2[i];
            binResult.vetItens[vetItens2[i]] = 1;
            binResult.vetPosItem[numItens] = Ponto(x, y, 0);
            binResult.vetRotacao[numItens] = Rotacao(int(i > tamVet));
            binResult.volumeOcupado += instanciaG.vetItens[vetItens2[i]].volume;
            binResult.demandaTotal  += instanciaG.vetItens[vetItens2[i]].peso;
            binResult.numItens += 1;
            numItens += 1;
            //std::cout<<x<<" "<<y<<"; "<<dx<<" "<<dy<<"\n";
            //std::cout<<instanciaG.vetItens[vetItens[i]].vetDim<<"\n\n";
        }

        if(!binResult.verificaViabilidade())
        {

            std::cout<<"CP-SAT Encontrou Solucao Viavel!\n";
            std::cout << "ERROR, Bin NAO eh Viavel!\n";
            PRINT_DEBUGG("", "");
            exit(-1);
        }

        criaEPs(binResult);
        //std::cout<<"CORRETO!\n";
        return true;
    }
    //std::cout<<"INCORRETO!\n";
    return false;

}

Resultado BinPackingCP_NS::testaCpSatBinPacking(int numItens)
{
    VectorI vetItens(numItens);
    vetItens.setAll(0);

    Vector<int8_t> vetItensSelecionados(instanciaG.numItens);
    vetItensSelecionados.setAll((int8_t)0);

    double volumeOcupado = 0.0;
    double volumeVeiculo = 1.0;
    double demanda       = 0.0;

    for(int i=0; i < instanciaG.numDim; ++i)
        volumeVeiculo *= instanciaG.vetDimVeiculo[i];


    for(int t=0; t < numItens; ++t)
    {
        int itemId = getRandInt(0, instanciaG.numItens-1);
        const int itemIdIni = itemId;
        while(vetItensSelecionados[itemId] == (int8_t)1 ||
             (volumeOcupado+instanciaG.vetItens[itemId].volume) > volumeVeiculo ||
             demanda + instanciaG.vetItens[itemId].peso > instanciaG.veicCap)
        {
            itemId = (itemId+1)%instanciaG.numItens;

            if(itemId == itemIdIni)
            {
                itemId = -1;
                break;
            }
        }

        if(itemId == -1)
        {
            numItens = t;
            break;
        }

        vetItensSelecionados[itemId] = (int8_t)1;
        volumeOcupado += instanciaG.vetItens[itemId].volume;
        demanda       += instanciaG.vetItens[itemId].peso;
        vetItens[t] = itemId;

    }

    //std::cout<<"\nNum itens: "<<numItens<<"\n\n";

    Bin binHeu;
    binHeu.reset();
    //binHeu.addEp(PontoZero);
    //binHeu.numEps = 1;

    std::sort(vetItens.begin(), vetItens.begin()+numItens);
    //std::cout<<vetItens<<"\n\n";

    //if(verificaInviabilidadePares(vetItens, numItens))
    {

        if(construtivoBinPacking(binHeu, vetItens, numItens, input.aphaBin, 10))
        {
            //std::cout << "Construtivo Encontrou Solucao Viavel!\n";
            if(!binHeu.verificaViabilidade())
            {
                //std::cout<<"Bin NAO eh Viavel\n";
                PRINT_DEBUGG("", "");
                exit(-1);
            }

            return HEURISTICA;
        }
        else
        {//std::cout<<"Construtivo NAO Encontrou Solucao Viavel!\n";


            Bin binCpSat;
            if(cpSatBinPacking2(binCpSat, vetItens, numItens) == true)
            {
                //std::cout<<"ret exato!\n";
                return EXATO;
            }
            else
                return INVIAVEL;


        }
    }


}

void BinPackingCP_NS::criaEPs(SolucaoNS::Bin &bin)
{

    for(int i=0; i < bin.numItens; ++i)
    {
        Ponto p    = bin.vetPosItem[i];
        Item& item = instanciaG.vetItens[bin.vetItemId[i]];

        Array<double, 3> vetD;

        for(int d=0; d < instanciaG.numDim; ++d)
            vetD[d] = item.getDimRotacionada(d, bin.vetRotacao[i]);

        for(int d=0; d < instanciaG.numDim; ++d)
        {
            Ponto ep(p);
            ep.vetDim[d] += vetD[d];
            bool colisao = false;

            for(int t=0; t < bin.numItens; ++t)
            {
                if(epColideItem(ep, bin.vetPosItem[t], bin.vetItemId[t]))
                {
                    colisao = true;
                    break;
                }
            }

            if(!colisao)
                bin.addEp(ep);
        }

    }

}

bool BinPackingCP_NS::verificaInviabilidadePares(VectorI& vetItens, int tamVet)
{

    for(int i=0; i < tamVet; ++i)
    {
        Item& itemI = instanciaG.vetItens[i];

        for(int j=(i+1); j < tamVet; ++j)
        {
            Item& itemJ = instanciaG.vetItens[i];
            bool empacotavel = false;

            for(int r0=0; r0 <= 1; ++r0)
            {
                double dim0ItemI = itemI.getDimRotacionada(0, (Rotacao)r0);
                double dim1ItemI = itemI.getDimRotacionada(1, (Rotacao)r0);

                for(int r1=0; r1 <= 1; ++r1)
                {
                    double dim0ItemJ = itemI.getDimRotacionada(0, (Rotacao)r0);
                    double dim1ItemJ = itemI.getDimRotacionada(1, (Rotacao)r0);

                    if((dim0ItemI+dim0ItemJ) <= instanciaG.vetDimVeiculo[0])
                    {
                        empacotavel = true;
                        break;
                    }

                    if((dim1ItemI+dim1ItemJ) <= instanciaG.vetDimVeiculo[1])
                    {
                        empacotavel = true;
                        break;
                    }

                }

                if(empacotavel)
                    break;
            }

            if(!empacotavel)
                return false;
        }

    }

    return true;

}

bool  BinPackingCP_NS::binPacking(VectorI &vetItens, int tamVet)
{

    Bin bin;
    bin.reset();


    if(construtivoBinPacking(bin, vetItens, tamVet, input.aphaBin, 10))
    {
            //std::cout << "Construtivo Encontrou Solucao Viavel!\n";
        if(!bin.verificaViabilidade())
        {
                //std::cout<<"Bin NAO eh Viavel\n";
            PRINT_DEBUGG("", "");
            exit(-1);
        }

            return true;
    }
    else
    {//std::cout<<"Construtivo NAO Encontrou Solucao Viavel!\n";
        bin.reset();
        return cpSatBinPacking2(bin, vetItens, tamVet);
    }


}


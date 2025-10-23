#ifndef GLOBALVARIABLES_H
#define GLOBALVARIABLES_H

#include <iostream>
#include <fstream>
#include "InputOutput.h"

namespace NS_GloabalVar
{
    inline int numAcertos       = 0;
    inline int numErros         = 0;
    inline int numTotal         = 0;
    inline int numItensAcerto   = 0;
    inline int numItensErro     = 0;
    inline int numAcertosBitSet = 0;

    inline void addAcerto(int itens)
    {
        numTotal += 1;
        numAcertos += 1;

        numItensAcerto += itens;
    }

    inline void addAcertoBitSet(int itens)
    {
        numTotal += 1;
        numAcertos += 1;
        numItensAcerto += 1;
        numAcertosBitSet += 1;
    }

    inline void addErro(int itens)
    {
        numTotal += 1;
        numErros += 1;

        numItensErro += itens;
    }

    inline void computaAcertosE_Erros()
    {
        double mediaItensErro   = (double)numItensErro/numErros;
        double mediaItensAcerto = (double)numItensAcerto/numAcertos;

        std::cout<<"\nTotal de Acertos: "<<numAcertos<<"\n";
        std::cout<<"Total de Erros: "<<numErros<<"\n";

        std::cout<<"Media Itens Acerto: "<<mediaItensAcerto<<"\n";
        std::cout<<"Media Itens Erro: "<<mediaItensErro<<"\n\n";

        std::cout<<"BitSet ajudou "<<numAcertosBitSet<<" vezes!\n";

        std::ofstream file("resultados.csv", std::ios::out|std::ios::app);
        file<<ParseInputNS::input.strInst<<"; "<<numAcertos<<"; "<<mediaItensAcerto<<"; "<<numErros<<"; "<<mediaItensErro<<"; "<<numAcertosBitSet<<"\n";
        file.close();

        std::cout<<ParseInputNS::input.strInst<<"\n";
    }
}

#endif // GLOBALVARIABLES_H

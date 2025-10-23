/* ****************************************
 * ****************************************
 *  Data:    05/11/24
 *  Arquivo: Solucao.h
 *  Autor:   Igor de Andrade Junqueira
 *  Projeto: 2L-SDVRP
 * ****************************************
 * ****************************************/

#ifndef INC_2L_SDVRP_SOLUCAO_H
#define INC_2L_SDVRP_SOLUCAO_H

#include "safe_vector.h"
#include "safe_matrix.h"
#include "Instancia.h"
#include "AuxT.h"


namespace SolucaoNS
{

    struct Ponto
    {
        Array<double,3> vetDim;

        Ponto(){vetDim.setAll(0.0);}
        Ponto(double d0, double d1, double d2)
        {
            vetDim[0] = d0;
            vetDim[1] = d1;
            vetDim[2] = d2;
        }

        std::string print()const;
    };

    struct Bin
    {

        //VectorD vetX, vetY;          // Indica aa posicoes do canto inferior esquerdo onde o item eh posicionado no bin
        //VectorD vetEpX, vetEpY;      // Indica as posicoes dos pontos extremos

        Vector<Ponto>                   vetPosItem;             // Indica as posicoes do canto inferior esquerdo onde o item eh posicionado no bin
        Vector<Ponto>                   vetEp;                  // Indica as posicoes dos pontos extremos
        VectorI                         vetItemId;              // Indica o id do iº item
        Vector<InstanciaNS::Rotacao>    vetRotacao;
        Vector<int8_t>                  vetItens;               // Indica se o bin empacota o i° item
        Array<double,3>                 binDim;
        double                          volumeTotal     = 0.0;
        double                          volumeOcupado   = 0.0;
        double                          demandaTotal    = 0.0;
        int                             numItens        = 0;
        int                             numEps          = 0;    // Numero de pontos extremos

        void addItem(int idEp, int idItem, InstanciaNS::Rotacao r);
        void addEp(const Ponto &ep);

        inline __attribute__((always_inline))
        void setItem(int pos, int itemId, double x, double y, double z)
        {

            #if VAR_SOLUTION_BORROW_CHECKER
                if(pos >= numItens)
                    throw std::out_of_range("");
            #endif

            vetPosItem[pos].vetDim[0]   = x;
            vetPosItem[pos].vetDim[1]   = y;
            vetPosItem[pos].vetDim[2]   = z;
            vetItemId[pos]              = itemId;

        };

        inline __attribute__((always_inline))
        void setEp(int pos, double x, double y, double z)
        {

            #if VAR_SOLUTION_BORROW_CHECKER
                if(pos >= numEps)
                    throw std::out_of_range("");
            #endif

            vetEp[pos].vetDim[0] = x;
            vetEp[pos].vetDim[1] = y;
            vetEp[pos].vetDim[2] = z;
        };

        inline __attribute__((always_inline))
        bool vazio() const
        {
            return numItens == 0;
        }

        bool rmI_Item(int i);
        bool rmI_Ep(int i);

        void reset();
        int getEpComMenorCoord(const VectorI &vetIdEp, int tam);
        bool verificaViabilidade();

        void rmItens(const VectorI &vetItensRm, const int tam);

        double getPorcentagemUtilizacao()const;

        Bin();
        Bin(const Bin &bin)=delete;
        Bin& operator=(const Bin &bin)=delete;
    };

    int getBinVazio(const Vector<Bin> &vetBin, int tam);
    void copiaBin(const Bin &binFonte, Bin &bin);

    class Rota
    {
    public:

        Rota();
        Rota(const Rota &rota)=delete;
        void reset();
        std::string printRota();

        VectorI        vetRota;
        VectorD        vetTempoSaida;
        VectorD        vetDemClie;
        Vector<int8_t> vetItens;        // Indica se o bin empacota o i° item
        int     numPos          = 2;
        //double  demTotal        = 0.0;
        double  distTotal       = 0.0;
        Bin*    binPtr          = nullptr;
    };

    class Solucao
    {
    public:

        Vector<Bin>     vetBin;
        Vector<Rota>    vetRota;
        double          distTotal = 0.0;

        Solucao()=default;
        explicit Solucao(const InstanciaNS::Instancia &instancia);

        bool verificaSol(std::string &error);
        int getBinVazio();
        void copiaSolucao(const Solucao &sol);
        void reset();
        double getUtilizacaoMediaBins()const;
        double getUtilizacaoMedianaBins()const;
        double getTamMedianaRota()const;


    };

    void copiaRota(const Rota &rotaFonte, Rota &rota);
    std::ostream& operator<<(std::ostream &os, const Solucao &sol);
    std::ostream& operator<<(std::ostream &os, const Bin& bin);

    std::string printBinEps(const Bin &bin);

    bool verificaColisaoDoisItens(int item0,
                                  int item1,
                                  const Ponto &p0,
                                  const Ponto &p1,
                                  InstanciaNS::Rotacao r0,
                                  InstanciaNS::Rotacao r1);

    std::string printPonto(const Ponto &ponto, int dim);

    int calculaNumBinOcupados(const Solucao &solucao);
    double calculaVolumeOcupado(const Solucao &solucao);
    double calculaVolumeLivre(const Solucao &solucao);
    double calculaMenorAreaLivre(const Solucao &solucao);

    inline __attribute__((always_inline))
    bool pontosIguais(const Ponto &p0, const Ponto &p1)
    {
        #pragma GCC unroll 3
        for(int i=0; i < 3; ++i)
        {
            if(p0.vetDim[i] != p0.vetDim[i])
                return false;
        }

        return true;
    }

    inline const Ponto PontoZero(0.0, 0.0, 0.0);
}

#endif //INC_2L_SDVRP_SOLUCAO_H

/* ****************************************
 * ****************************************
 *  Data:    05/11/24
 *  Arquivo: Instancia.h
 * ****************************************
 * ****************************************/

#ifndef INC_2L_SDVRP_INSTANCIA_H
#define INC_2L_SDVRP_INSTANCIA_H

#include <iostream>
#include <string>
#include "safe_vector.h"
#include "safe_matrix.h"
#include "sefe_array.h"
#include "Constantes.h"
#include "string"

namespace InstanciaNS
{
    enum Rotacao
    {
        Rot0 = 0,
        Rot1
    };

    class Item
    {
    public:

        Array<double,3> vetDim;    // Comprimento, largura, altura
        double volume = 0.0;
        double peso   = 0.0;

        Item()=default;
        Item(double x, double y, double z, double peso_);
        void set(double x, double y, double z);
        std::string print(bool printVol=false);
        double getDimRotacionada(int d, Rotacao r);

    };

    struct TW
    {
        double ini=0.0, fim=INF_Double;
    };

    class Instancia
    {
    public:

        std::string nome;
        int numClientes = 0;
        int numItens    = 0;
        Vector<int> vetNumItensPorCli;
        int numVeiculos = 0;
        int numDim = 2;

        double veicCap = 0;

        Array<double,3> vetDimVeiculo;
        //double veicAltura = 0.0;
        //double veicLargura = 0.0;

        bool split      = false;
        bool packing    = true;

        Matrix<double> matDist;
        Matrix<double> matTempo;
        Vector<TW> vetTw;

        Vector<Item> vetItens;
        VectorD vetPesoItens;
        VectorD vetMinDimItens;

        int maxNumItensPorClie = 0;


        //Vector<double> vetItemAltura;
        //Vector<double> vetItemLargura;
        //Vector<double> vetItemArea;

        Vector<double> vetDemandaCliente;

        Vector<int> vetItemCliente;      // Indica o cliente dado um item; mat[itemId] = cliente
        Matrix<int> matCliItensIniFim;   // Indica o id Inicial e id Final do primeiro e ultimo item de um cliente; mat[clienteId,0] = iten0; mat[clienteId,1] = itenFim

        Instancia();
        Instancia(int numClientes_, int numItens_, int numVeiculos_);
        void atualizaVetMinDimItens();

    };

    void leInstancia(const std::string &strFile);
    int copiaItensCliente(int cliente, VectorI& vetItens);
    int copiaItensClientes(VectorI& vetClientes, int tam, VectorI& vetItens);
    double calculaDistancia(VectorI& vet, int tam);
    inline Instancia instanciaG;

    //std::string printItem(int itemId);
}
// 35 s
// 47 s
#endif //INC_2L_SDVRP_INSTANCIA_H

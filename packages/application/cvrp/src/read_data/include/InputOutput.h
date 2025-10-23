/* ****************************************
 * ****************************************
 *  Data:    05/11/24
 *  Arquivo: ParseInput.h
 * ****************************************
 * ****************************************/

#ifndef INC_2L_SDVRP_INPUTOUTPUT_H
#define INC_2L_SDVRP_INPUTOUTPUT_H

#include "string"
#include <chrono>
#include "rand.h"
#include <iostream>
#include "Solucao.h"

namespace ParseInputNS
{
    class File
    {
    public:

        std::string fileSol;            // X
        std::string fileNum;            // X
        std::string fileResulCSV;       // X
        std::string fileSolPrint;
        std::string fileSeed;           // X
    };

    class Input
    {
    public:

        std::string strInstCompleto;
        std::string strInst;
        bool        splitInstancia           = false;
        bool        splitVrp                 = false;
        double      aphaBin                  = 0.3;
        double      aphaBinEscolhaEp         = 0.3;
        double      alphaVrp                 = 0.3;
        int         numItIG                  = 2000;
        double      gapIgReset               = 0.20;
        bool        comprimentoAlturaIguais1 = false;
        bool        cpSat                    = true;
        double      cpSatTime                = -1.0;
        // 0 Forward, 1 Backard, 2 Bidirectional
        int         labelingType             = 0;

        File file;
        std::string commit = "54f84fa9027eeb1a17566368b19204f726b1e4d0";


    };

    class Output
    {
    public:
        std::string  data;
        std::string  fileSaida;
        unsigned int semente;
        double       tempoCpu = 0.0;
        double       tempoRelogio = 0.0;

        void setup()
        {
            semente = RandNs::estado_;
            std::time_t result = duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            data = std::string(std::asctime(std::localtime(&result)));
        }
    };

    inline Input input;
    inline Output output;

    void parseInput(int argc, const char* argv[]);

    void escreveFileNum(const std::string &fileNum);
    void escreveFileResulCsv(const SolucaoNS::Solucao &sol, const std::string &fileResulCSV, double tempoCpu);
    void escreveFileSeed(const std::string &fileSeed);
    void escreveFileSol(const SolucaoNS::Solucao &sol, const std::string &fileSol);
    void escreveFileSolPrint(const SolucaoNS::Solucao &sol, const std::string &fileSol);
    void escreveSaidas(const SolucaoNS::Solucao &best, double tempoCpu);


    std::string getNomeInstancia(std::string str);

}

#endif //INC_2L_SDVRP_INPUTOUTPUT_H

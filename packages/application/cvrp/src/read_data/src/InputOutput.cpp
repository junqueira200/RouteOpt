/* ****************************************
 * ****************************************
 *  Data:    05/11/24
 *  Arquivo: ParseInput.cpp
 * ****************************************
 * ****************************************/

#include "InputOutput.h"
#include <memory>
#include "cxxopts.hpp"
#include <iostream>
#include "AuxT.h"
#include "rand.h"
#include <fstream>
#include <filesystem>

using namespace ParseInputNS;

void ParseInputNS::parseInput(int argc, const char* argv[])
{

    try
    {
        std::unique_ptr<cxxopts::Options> allocated(new cxxopts::Options(argv[0], " - example command line options"));
        auto& options = *allocated;
        options
                .positional_help("[optional args]")
                .show_positional_help();

        options
                .set_width(70)
                .set_tab_expansion()
                .allow_unrecognised_options()
                .add_options()
                        ("i,inst", "inst", cxxopts::value<std::string>(input.strInstCompleto))
                        //("s,split", "Indicate if the instance allows split delevery", cxxopts::value<bool>(input.splitInstancia))
                        ("alphaBin", "Value of the aplha parameter for the contrutive algorithm for bin packing", cxxopts::value<double>(input.aphaBin))
                        //("aphaBinEp", "Controla a escolha do EP", cxxopts::value<double>(input.aphaBinEscolhaEp))
                        //("saida", "Resultado do bin packing", cxxopts::value<std::string>(output.fileSaida))
                        ("seed", "Seed of the random number generator", cxxopts::value<unsigned int>(output.semente))
                        ("alphaVrp", "Value of the aplha parameter for the contrutive algorithm for VRP", cxxopts::value<double>(input.alphaVrp))
                        ("it", "Number of iterations of IG", cxxopts::value<int>(input.numItIG))
                        //("fileNum", "Contem o numero de vezes em que a instancia foi executada com sucesso", cxxopts::value<std::string>(input.file.fileNum))
                        //("fileSeed", "Arquivo em que a semente eh salva", cxxopts::value<std::string>(input.file.fileSeed))
                        ("resulCSV", "Arquivo em que sao escritos os resultados de cada execucao", cxxopts::value<std::string>(input.file.fileResulCSV))
                        //("fileSol", "Arquivo em que eh escrito a solucao completa", cxxopts::value<std::string>(input.file.fileSol))
                        //("comprimentoAlturaIguais1", "Desabilita o bin packing, fazendo largura e altura de todos os itens iguais a 1", cxxopts::value<bool>(input.comprimentoAlturaIguais1))
                        //("cpSat", "Abilita o cp-sat no empacotamento", cxxopts::value<bool>(input.cpSat))
                        //("cpSatTime", "Tempo limite para o cp-sat", cxxopts::value<double>(input.cpSatTime))
                        ("lTaype", "Labeling Type (0, 1, 2)", cxxopts::value<int>(input.labelingType));

        auto result = options.parse(argc, argv);

        if (result.count("help"))
        {
            std::cout << options.help({"", "Group"}) << std::endl;
            std::cout<<"Commit: "<<input.commit<<"\n";
            exit(-1);
        }

        if(result.count("inst") == 0)
        {
            std::cout << options.help({"", "Group"}) << std::endl;
            std::cout<<"Commit: "<<input.commit<<"\n";
            assertm(true, "Instancia (-f) nao foi detectada na entrada");
        }


        RandNs::startEngine(output.semente, bool(result.count("seed")==1));

    }
    catch (const cxxopts::exceptions::exception& e)
    {
        std::cout << "error parsing options: " << e.what() <<    std::endl;
        throw e;
    }

    input.strInst = getNomeInstancia(input.strInstCompleto);

}

void ParseInputNS::escreveFileNum(const std::string &fileNum)
{
    if(fileNum.empty())
        return;

    int num = 0;

    if(std::filesystem::exists(fileNum))
    {
        std::ifstream fileLe;
        fileLe.open(fileNum);


        if(!fileLe.is_open())
        {
            std::cout << "Error, nao foi possivel abrir o arquivo: " << fileNum << "\n";
            throw std::ifstream::failure(fileNum);
        }

        fileLe >> num;
        fileLe.close();
    }

    std::ofstream file;
    file.open(fileNum, std::ios::out);

    if(!file.is_open())
    {
        std::cout<<"Error, nao foi possivel abrir o arquivo: "<<fileNum<<"\n";
        throw std::ifstream::failure(fileNum);
    }

    num += 1;

    file<<num<<"\n";
    file.close();

}

void ParseInputNS::escreveFileResulCsv(const SolucaoNS::Solucao &sol, const std::string &fileResulCSV, double tempoCpu)
{

    std::ofstream file;

    if(!std::filesystem::exists(fileResulCSV))
    {
        file.open(fileResulCSV, std::ios_base::out);

        if(!file.is_open())
        {

            std::cout << "Error, nao foi possivel abrir o arquivo: " << fileResulCSV << "\n";
            throw std::ifstream::failure(fileResulCSV);
        }

        file<<"dist, tempoCpu, utilMediana, tamRotaMediana, seed\n";
    }
    else
        file.open(fileResulCSV, std::ios_base::app);

    if(!file.is_open())
    {

        std::cout << "Error, nao foi possivel abrir o arquivo: " << fileResulCSV << "\n";
        throw std::ifstream::failure(fileResulCSV);
    }

    file<<sol.distTotal<<", "<<tempoCpu<<", "<<sol.getUtilizacaoMedianaBins()<<", "<<sol.getTamMedianaRota()<<", "<<output.semente<<"\n";
    file.close();


}

void ParseInputNS::escreveFileSeed(const std::string &fileSeed)
{

    if(fileSeed.empty())
        return;

    std::ofstream seedFile;
    if(!std::filesystem::exists(fileSeed))
    {
        seedFile.open(fileSeed, std::ios_base::out);
        seedFile<<"seed\n";
    }
    else
        seedFile.open(fileSeed, std::ios_base::app);

    seedFile<<std::to_string(output.semente)<<"\n";

}

void ParseInputNS::escreveFileSol(const SolucaoNS::Solucao &sol, const std::string &fileSol)
{

    if(fileSol.empty())
        return;

    std::ofstream file;
    file.open(fileSol, std::ios_base::out);

    file << output.data << "\n" << input.strInstCompleto << "\n\n";
    file<<sol;
    file.close();
}

void ParseInputNS::escreveFileSolPrint(const SolucaoNS::Solucao &sol, const std::string &fileSol)
{

    if(fileSol.empty())
        return;
}

void ParseInputNS::escreveSaidas(const SolucaoNS::Solucao &best, double tempoCpu)
{
    escreveFileNum(input.file.fileNum);
    escreveFileResulCsv(best, input.file.fileResulCSV, tempoCpu);
    escreveFileSeed(input.file.fileSeed);
    escreveFileSol(best, input.file.fileSol);
    escreveFileSolPrint(best, input.file.fileSolPrint);
}

std::string ParseInputNS::getNomeInstancia(std::string str)
{
    int posNome = -1;

    for(int i = 0; i < str.size(); ++i)
    {
        if(str[i] == '/')
            posNome = i + 1;
    }

    if(posNome < str.size())
    {
        std::string nome = str.substr(posNome);

        int posPonto = -1;

        for(int i = 0; i < nome.size(); ++i)
        {
            if(nome[i] == '.')
            {
                posPonto = i - 1;
                break;
            }

//delete bestSol;
        }

        if(posPonto > 0)
        {   //cout<<"posNome: "<<posNome<<"\n\n";
            return nome.substr(0, (posPonto + 1));
        } else
            return nome;
    } else
        return "ERRO";

}

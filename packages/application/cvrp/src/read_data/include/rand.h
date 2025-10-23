/* ****************************************
 * ****************************************
 *  Data:    11/11/24
 *  Arquivo: rand.h
 *  Autor:   Igor de Andrade Junqueira
 *  Projeto: 2L-SDVRP
 * ****************************************
 * ****************************************/

#ifndef TTP_RAND_H
#define TTP_RAND_H

#include <iostream>
#include <random>

namespace RandNs
{
    //std::ranlux48 ranlux48_;
    inline std::mt19937_64 engine_;
    //inline std::ranlux24 engine_;
    inline bool start_ = false;
    inline unsigned int estado_ = 0;

    unsigned int startEngine(unsigned int rd_, bool validoRd=false);

    // Val aleatorio entre [ini; fim]
    inline __attribute__((always_inline))
    int getRandInt(const int ini, const int fim)
    {
        std::uniform_int_distribution unif{ini,fim};
        return unif(engine_);
    }

    // Val aleatorio entre [0; 1)
    inline __attribute__((always_inline))
    double getRandDouble()
    {
        std::uniform_real_distribution unif;
        return unif(engine_);
    }

    // val aleatorio entre [ini; fim)
    inline __attribute__((always_inline))
    double getRandDouble(const double ini, const double fim)
    {
        std::uniform_real_distribution unif(ini, fim);
        return unif(engine_);
    }

}
#endif //TTP_RAND_H
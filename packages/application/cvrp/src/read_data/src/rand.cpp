/* ****************************************
 * ****************************************
 *  Data:    22/03/23
 *  Arquivo: rand.cpp
 *  Autor:   Igor de Andrade Junqueira
 *  Projeto: 2L-SDVRP
 * ****************************************
 * ****************************************/

#include "rand.h"

unsigned int RandNs::startEngine(unsigned int rd_, bool validoRd)
{
    if(!start_)
    {
        if(!validoRd)
        {
            unsigned int rd = std::random_device{}();
            estado_ = rd;
            engine_ = std::mt19937_64(rd);
            //engine_ = std::ranlux24(rd);
            start_ = true;
            return rd;
        } else
        {
            estado_ = rd_;
            engine_ = std::mt19937_64(rd_);
            //engine_ = std::ranlux24(rd_);
            start_ = true;
            return rd_;
        }
    }

    return estado_;
}
/* ****************************************
 * ****************************************
 *  Nome: ***********************
 *  Data:    31/10/22
 *  Arquivo: VetorHash.h
 * ****************************************
 * ****************************************/

#ifndef INC_2E_EVRP_VETORHASH_H
#define INC_2E_EVRP_VETORHASH_H

#include "safe_vector.h"
#include <unordered_set>
#include <iostream>
#include <algorithm>
#include <fstream>

namespace NS_Igor_HashVector
{

    class HashVector
    {
    public:

        VectorI     vet;   // Vetor guarda a rota ou a solucao
        std::size_t valHash = 0;

        explicit HashVector(const VectorI &vet)
        {
            this->vet = vet;
            std::sort(this->vet.begin(), this->vet.end());
            computeHashValue();
        }

        HashVector()=default;

        void computeHashValue()
        {
            // https://cseweb.ucsd.edu/~kube/cls/100/Lectures/lec16/lec16-16.html
            // ELF Hash algorithms
            valHash = 0;
            for(int i=0; i < vet.size(); ++i)
            {   //         valHash * 16
                valHash = (valHash<<4) + vet[i];
                size_t g = valHash & 0xF0000000L;

                if(g != 0)
                    valHash ^= g >> 24;
                valHash &= ~g;
            }
        }


        void print() const
        {
            if(valHash == 0)
            {
                std::cout<<"vet empty\n";
                return;
            }

            std::cout<<"vetSize: "<<vet.size()<<"\n";
            std::cout<<"VET: ";
            for(int t:vet)
                std::cout<<t<<" ";

            std::cout<<"\nHASH: "<<valHash<<"\n\n";
        }

        bool operator ==(const HashVector &vetorHash) const
        {
            if(valHash != vetorHash.valHash)
                return false;

            if(vet.size() != vetorHash.vet.size())
                return false;

            for(int i=0; i < vet.size(); ++i)
            {
                if(vet[i] != vetorHash.vet[i])
                    return false;
            }

            return true;
        }

    };


    class HashFunc
    {
    public:

        size_t operator()(const HashVector &vetorHash) const
        {
            return vetorHash.valHash;
        }
    };

    inline void writeSetToFile(std::unordered_set<NS_Igor_HashVector::HashVector, NS_Igor_HashVector::HashFunc>& ig_hashPacking,
                        std::string&& file)
    {
        std::fstream fileFs;
        fileFs.open(file, std::ios::out);

        for(auto& hash:ig_hashPacking)
        {
            fileFs<<hash.vet<<"\n";
        }

        fileFs.close();

    }

    inline void writeVectorOfRoutesToFile(std::vector<std::vector<int>>& vetRoutes, std::string&& file)
    {
        std::cout<<"Ini\n";
        std::fstream fileFs;
        fileFs.open(file, std::ios::out);
        std::cout<<"size: "<<vetRoutes.size()<<"\n";

        for(auto &vet:vetRoutes)
        {
            for(int i=0; i < vet.size()-1; ++i)
                fileFs<<vet[i]<<" ";

            fileFs<<vet[vet.size()-1]<<"\n";
            fileFs.close();
            std::cout<<"END\n";
            exit(-1);
        }

        fileFs.close();
    }

}

#endif //INC_2E_EVRP_VETORHASH_H

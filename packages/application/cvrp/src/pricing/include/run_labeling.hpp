/* 
 * Copyright (c) 2025 Zhengzhong (Ricky) You.
 * All rights reserved.
 * Software: RouteOpt
 * License: GPL-3.0
 */

#ifndef ROUTE_OPT_RUN_LABELING_HPP
#define ROUTE_OPT_RUN_LABELING_HPP
#include "solver.hpp"
#include "solver_macro.hpp"
#include "pricing_macro.hpp"
#include "dominance.hpp"
#include "cvrp_pricing_controller.hpp"


#include "InputOutput.h"
#include "Instancia.h"
#include "ConstrutivoBin.h"
#include "Construtivo.h"
#include "AuxT.h"
#include "rand.h"
#include "Ig.h"
#include "BinPackingCP.h"
#include "VetorHash.h"
#include "GlobalVariables.h"

using namespace InstanciaNS;
using namespace ConstrutivoBinNS;
using namespace SolucaoNS;
using namespace RandNs;
using namespace ParseInputNS;
using namespace ConstrutivoNS;
using namespace IgNs;
using namespace BinPackingCP_NS;
using namespace NS_GloabalVar;
using namespace RouteOpt::Application::CVRP;


inline void IG_addCstr(const sparseRowMatrixXd &mat, std::vector<char> &sense,
                            std::vector<double> &rhs, RouteOpt::Solver &solver) {
            size_t nz = mat.nonZeros();
            std::vector<size_t> solver_beg(mat.rows() + 1);
            std::vector<int> solver_ind(nz + sense.size());
            std::vector<double> solver_val(nz + sense.size());

            nz = 0;
            solver_beg[0] = 0;
            for (int k = 0; k < mat.outerSize(); ++k) {
                sparseRowMatrixXd::InnerIterator it(mat, k);
                if (it) {
                    if (RouteOpt::equalFloat(rhs[k], 0)) {
                        if (it.col() != 0) {
                            solver_ind[nz] = static_cast<int>(it.col());
                            solver_val[nz] = it.value();
                            ++nz;
                        }
                    } else {
                        solver_ind[nz] = 0;
                        solver_val[nz] = rhs[k];
                        ++nz;
                        if (it.col() != 0) {
                            solver_ind[nz] = static_cast<int>(it.col());
                            solver_val[nz] = it.value();
                            ++nz;
                        }
                    }
                }

                ++it;

                for (; it; ++it) {
                    solver_ind[nz] = static_cast<int>(it.col());
                    solver_val[nz] = it.value();
                    ++nz;
                }
                solver_beg[k + 1] = nz;
            }

            solver_ind.resize(nz);
            solver_val.resize(nz);


            SAFE_SOLVER(solver.XaddConstraints(
                mat.rows(),
                nz,
                solver_beg.data(),
                solver_ind.data(),
                solver_val.data(),
                sense.data(),
                rhs.data(),
                nullptr
            ))
            SAFE_SOLVER(solver.updateModel())

}


namespace RouteOpt::Application::CVRP {
    template<bool dir, bool if_last_half, bool if_complete, bool if_symmetry, PRICING_LEVEL pricing_level>
    void CVRP_Pricing::runLabeling(double time_limit, BbNode *node) {
        if_stop_arc_elimination = pricing_level != PRICING_LEVEL::EXACT;
        if_exact_labeling_cg = pricing_level == PRICING_LEVEL::EXACT;
        if_exact_labeling_finished = pricing_level == PRICING_LEVEL::EXACT;
        if_short_memory = false;


        bool if_suc;
        if constexpr (!if_last_half) {
            if constexpr (dir) {
                initializeLabels<dir, if_symmetry, true, true, true>();
            } else {
                initializeLabels<dir, if_symmetry, false, true, true>();
            }
        }

        auto beg = std::chrono::high_resolution_clock::now();

        if constexpr (CHECK_PRICING_LABELS) {
            inner_bin_len = {0, 0};
            outer_bin_len = {0, 0};
            outer_bin_but_keep_len = {0, 0};
        }
        int min_sorted_b = dir ? -1 : num_buckets_per_vertex;
        for (int b = (dir ? 0 : num_buckets_per_vertex - 1); (dir ? b < num_buckets_per_vertex : b >= 0); (
                 dir ? ++b : --b)) {
            for (auto &comp: dir ? topological_order_forward_ptr->at(b) : topological_order_backward_ptr->at(b)) {
                int index = 0;
            STILL_EXIST:
                for (; index < comp.size(); ++index) {
                    int i = comp[index];
                    auto &valid_num =
                            dir
                                ? if_exist_extra_labels_in_forward_sense[i][b].second
                                : if_exist_extra_labels_in_backward_sense[i][b].second;
                    if (!valid_num) continue;
                    auto &label_array =
                            dir
                                ? if_exist_extra_labels_in_forward_sense[i][b].first
                                : if_exist_extra_labels_in_backward_sense[i][b].first;
                    for (int vec_index = 0; vec_index < valid_num; ++vec_index) {
                        auto &ki = label_array[vec_index];
                        if (ki->is_extended) continue;
                        checkIfDominated<dir, pricing_level>(ki, i, b, if_suc);
                        ki->is_extended = true;
                        if (!if_suc)continue;
                        auto sig = extendKernel4Exact<int,
                            dir, if_last_half, if_complete, if_symmetry, pricing_level>(
                            ki,
                            i,
                            ki->res,
                            dir
                                ? all_forward_buckets[i][b].bucket_arcs
                                : all_backward_buckets[i][b].bucket_arcs);

                        if (sig == PRICING_STATE::OUT_OF_MEMORY) {
                            goto populateBin;
                        }

                        sig = extendKernel4Exact<std::pair<res_int, int>,
                            dir, if_last_half, if_complete, if_symmetry, pricing_level>(ki, i, ki->res,
                            dir
                                ? all_forward_buckets[i][b].jump_arcs
                                : all_backward_buckets[i][b].jump_arcs);

                        if (sig == PRICING_STATE::OUT_OF_MEMORY) {
                            goto populateBin;
                        }
                    }
                    valid_num = 0;
                }
                for (index = 0; index < comp.size(); ++index) {
                    int i = comp[index];
                    if (dir
                            ? if_exist_extra_labels_in_forward_sense[i][b].second
                            : if_exist_extra_labels_in_backward_sense[i][b].second) {
                        goto STILL_EXIST;
                    }
                }
            }
            for (int i = 1; i < dim; ++i) sortLabelsInBinByRC<dir>(i, b);
            min_sorted_b = b;

            if (std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - beg).count() > time_limit) {
                if_exact_labeling_finished = false;
                goto QUIT;
            }
        }


    populateBin:

        for (int i = 1; i < dim; ++i) {
            for (int b = dir ? min_sorted_b + 1 : min_sorted_b - 1; dir ? b < num_buckets_per_vertex : b >= 0;
                 dir ? ++b : --b) {
                sortLabelsInBinByRC<dir>(i, b);
            }
        }

    QUIT:
        if (if_short_memory)
            PRINT_WARNING("allocate more memory");
        if constexpr (CHECK_PRICING_LABELS) {
            if (if_exact_labeling_cg) {
                PRINT_DEBUG("check labels");
                std::cout << "inner= " << inner_bin_len.first / inner_bin_len.second << " nominal= " << inner_bin_len.
                        second << " " <<
                        "outer= " << outer_bin_len.first / outer_bin_len.second << " nominal= " << outer_bin_len.second
                        << " " <<
                        "outer2= " << outer_bin_but_keep_len.first / outer_bin_but_keep_len.second << " nominal= "
                        << outer_bin_but_keep_len.second << std::endl;
                std::cout << "num_dominance_checks= " << num_dominance_checks << std::endl;
            }
        }

        // TODO: Chamar o bin packing aqui!
        const std::vector<SequenceInfo>& vet = getNewCols();
        bool cutAdd = false;

        for(auto seqInfo:vet)
        {
            std::bitset<200> cust = 0;
            for(auto cli:seqInfo.col_seq)
            {
                //std::cout<<cli<<" ";
                cust[cli] = true;
            }

            VectorI vetItens;
            VectorI vetCust;
            double wight = 0.0;
            vetItens.reserve(10);

            for(int i=1; i < 200; ++i)
            {
                if(cust[i] == false)
                    continue;
                vetCust.push_back(i);
                int ini = instanciaG.matCliItensIniFim(i, 0);
                int fim = instanciaG.matCliItensIniFim(i, 1);

                if(fim > instanciaG.numItens)
                {
                    std::cout<<"Error!";
                    std::cout<<"Num de itens errado!\n";
                    PRINT_DEBUGG("", "");
                    throw "ERROR";
                }

                for(int item=ini; item <= fim; ++item)
                {
                    vetItens.push_back(item);
                    wight += InstanciaNS::instanciaG.vetItens[item].peso;
                }
            }

            //std::cout<<"\n";
            std::sort(vetItens.begin(), vetItens.end());
            NS_Igor_HashVector::HashVector hashV(vetItens);

            bool callPacking = true;
            bool packaged    = false;

            //Ig_BitSet bitSet = 0;
            //ig_createBitSet(vetCust, bitSet);

            if(ig_hashPacking.count(hashV) == 0)
            {
                if(ig_hashNoPacking.count(hashV) == 1)
                {
                    callPacking = false;
                    // ADD CUT!
                }
                else
                    callPacking = true;

            }
            else
            {
                callPacking = false;
                packaged    = true;
            }

            /*
            if(callPacking)
            {
                callPacking = false;
                callPacking = !ig_packingChecking(bitSet, wight);
                if(!callPacking)
                {	std::cout<<"bit set ajudou!\n\n";
                    addAcertoBitSet(vetItens.size());
                }

            }
            */

            if(callPacking)
            {
                packaged = binPacking(vetItens, vetItens.size());
                //std::cout<<"call packing!\n";
                if(packaged)
                {
                    ig_hashPacking.insert(hashV);
                    //ig_vetRoutes.push_back(std::vector<int>());
                    //std::vector<int>& vetI = ig_vetRoutes[ig_vetRoutes.size()-1];
                    //for(int item:vetItens)
                    //    vetI.push_back(item);

                    addAcerto(vetItens.size());
                    //ig_addBitSet(bitSet, wight);
                }
                else
                {
                    ig_hashNoPacking.insert(hashV);
                    addErro(vetItens.size());
                    // Add a rcc cut

                    if(node)
                    {
                    cutAdd = true;
                    std::cout<<"Starting cut route: "<<vetCust<<"\n";
                    RCCs::Rcc rcc;
                    for(int cust:vetCust)
                        rcc.info_rcc_customer.push_back(cust);


                    rcc.if_keep = true;
                    rcc.rhs = vetCust.size() - 1;
                    rcc.form_rcc = (int)RCCs::RCCForm::RCC_FORM_1;

                    std::vector<RCCs::Rcc> vetRcc;
                    std::vector<double> rhs;
                    rhs.push_back(vetCust.size()-1);

                    std::vector<char> sense;
                    sense.push_back(SOLVER_LESS_EQUAL);


                    vetRcc.push_back(rcc);
                    sparseRowMatrixXd matRccCoef;
                    RCCs::CoefficientGetter::RCCCoefficientController::
                    getCoefficientRCC(node->getCols(), vetRcc, false, matRccCoef);

                    IG_addCstr(matRccCoef, sense, rhs, node->refSolver());

                    std::cout<<"Route cuted: "<<vetCust<<"\n\n";
                    SAFE_SOLVER(node->refSolver().reoptimize(SOLVER_DUAL_SIMPLEX));
                    }

                }
            }
        }

    }

    template<bool if_symmetry>
    int CVRP_Pricing::concatenateCols_prior_forward() {
        double tmp_rc;
        int i, j, arr_bj;
        int if_state;

        for (auto &label_list: concatenate_labels_in_forward_cg) {
            i = label_list.first.first;
            j = label_list.first.second;
            auto &label_vec = label_list.second;
            for (auto &pr: label_vec) {
                auto &ki = pr.first;
                auto &tmp_Resource = pr.second;
                arr_bj =
                        if_symmetry
                            ? static_cast<int>((resource.resources[0] - tmp_Resource.resources[0]) / step_size)
                            : tmp_Resource.resources[0]
                              / step_size;
                tmp_rc = ki->rc + chg_cost_mat4_vertex[i][j];

                for (; if_symmetry ? arr_bj >= 0 : arr_bj < num_buckets_per_vertex;
                       if_symmetry ? --arr_bj : ++arr_bj) {
                    concatenateOneLabelWithOtherLabels<true, if_symmetry, false>(ki,
                        j,
                        arr_bj,
                        tmp_rc,
                        tmp_Resource,
                        if_state);
                    if (if_state == static_cast<int>(CONCATENATE_STATE::TOTALLY_FAIL))break;
                }
            }
        }

        writeColumnsInPricingPool();

        return static_cast<int>(new_cols.size());
    }
}

#endif // ROUTE_OPT_RUN_LABELING_HPP

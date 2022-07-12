#ifndef SLABC_H
#define SLABC_H

#include <string>
#include <vector>
#include <random>
#include <memory>
#include <fstream>
#include <thread>
#include <atomic>

#include "../../include/rouflex/instance.h"
#include "../../include/rouflex/encoding.h"
#include "../../include/rouflex/decoder.h"

namespace rouflex {

    class RLActionProvider {
    public:
        //I just need a dump default constructor
        RLActionProvider() : learning_rate{0.75}, discount_factor{0.25}, eps{0.3} {}
        //This is the real constructor which shall be invoked
        RLActionProvider(   const int n_states, const int n_actions, const float eps,
                            const double learning_rate, const double discount_factor);
        int getEGreedyAction(const int state, std::default_random_engine & reng) const;
        inline int getBestActionAtState(const int state) const {
            return this->best_action_by_state[ state ];
        }
        void updateByRewardQL(const int state, const int action, const int new_state, const double reward);
        inline int getNStates() const {
            return this->Qsa_matrix.size();
        }
    private:
        void updateBestAction(const int state, const int action);
        std::vector< std::vector<double> > Qsa_matrix;
        std::vector< int > best_action_by_state;
        double learning_rate;
        double discount_factor;
        int n_actions;
        float eps;
    };

    class SLABC {
    public:
        SLABC(
            const Instance & instance_inp,
            const unsigned int nIter_inp,
            const unsigned int nEP_inp,
            const unsigned int nOP_inp,
            const int limit_inp,
            const float Phyb_init_inp,
            const float eps_inp,
            const double learning_rate_inp,
            const double discount_factor_inp,
            const double no_improvement_reward_inp,
            std::default_random_engine & rengine_inp,
            const std::vector< double > & state_score_thresholds_inp,
            const std::vector< float > & Pmpi_vector_inp,
            const std::vector< int > & nrp_vector_inp,
            const std::vector< int > & nswp_vector_inp,
            const std::vector< int > & nmc_vector_inp,
            const std::vector< float > & Phyb_vector_inp,
            const std::string & stats_dump_path_inp
        );
        //method to launch the solver
        inline timeperiod::timeunit solve() {
            for(int iter = 0; iter < this->nIter; ++iter) this->iterate();
            this->saveStats();
            return this->getBestMakespan();
        }
        inline timeperiod::timeunit getBestMakespan() const {
            return this->best_makespan;
        }
        inline int getNFoodSources() const {
            return this->food_sources.size();
        }
    private:
        const Instance & instance;
        const unsigned int nIter;
        const unsigned int nOP; //Number of Onlooker Bees
        const int limit; //limit after which we substitute a solution
        const float Phyb_init;
        const float eps;
        const double learning_rate;
        const double discount_factor;
        const double no_improvement_reward;
        std::default_random_engine & rengine;
        Decoder dec;
        inline int getNStates() const { return this->fitness_state_score_thresholds.size() + 1; }
        std::vector< std::unique_ptr<Encoding> > food_sources; //This vector shall contain food sources encodings
        std::vector< int > no_improvement_cycles;   //This vector shall record for each food source
                                                    //the amount of iterations passed without an improvement
        void exploreFoodSource(const int fs_index);
        int binaryTournamentFoodSource() const;
        timeperiod::timeunit best_makespan;
        std::vector< double > fitness_state_score_thresholds;
        double getFitnessStateScore(const timeperiod::timeunit fitness);
        int getFitnessState(const timeperiod::timeunit fitness);
        inline double getReward(const timeperiod::timeunit original_fitness, const timeperiod::timeunit new_fitness) {
            if(new_fitness <= original_fitness) return original_fitness - new_fitness;
            return no_improvement_reward;
        }
        void buildPmpiActions(const std::vector< float > & Pmpi_vector_inp);
        void buildCrossActions(const std::vector< int > & nrp_vector_inp);
        void buildMutActions(   const std::vector< int > & nswp_vector_inp,
                                const std::vector< int > & nmc_vector_inp,
                                const std::vector< float > & Phyb_vector_inp);
        RLActionProvider PmpiActionProvider;
        RLActionProvider CrossActionProvider;
        RLActionProvider MutActionProvider;
        std::vector< float > Pmpi_by_action;
        std::vector< int > nrp_by_action;
        std::vector< int > nswp_by_action;
        std::vector< int > nmc_by_action;
        std::vector< float > Phyb_by_action;
        int getPmpiAction(const int state) const;
        int getCrossAction(const int state) const;
        int getMutAction(const int state) const;
        inline float getPmpiFromPmpiAction(const int Pmpi_action) const {
            assert( this->Pmpi_by_action.size() > Pmpi_action );
            return this->Pmpi_by_action[Pmpi_action];
        }
        inline int getNrpFromCrossAction(const int cross_action) const {
            assert( this->nrp_by_action.size() > cross_action );
            return this->nrp_by_action[cross_action];
        }
        inline int getNswpFromMutAction(const int mut_action) const {
            assert( this->nswp_by_action.size() > mut_action );
            return this->nswp_by_action[mut_action];
        }
        inline int getNmcFromMutAction(const int mut_action) const {
            assert( this->nmc_by_action.size() > mut_action );
            return this->nmc_by_action[mut_action];
        }
        inline float getPhybFromMutAction(const int mut_action) const {
            assert( this->Phyb_by_action.size() > mut_action );
            return this->Phyb_by_action[mut_action];
        }
        void updateFoodSource(const int fs_index, std::unique_ptr<Encoding>& candidate_food_source);
        inline void updateBestMakespan(const timeperiod::timeunit new_makespan) {
            this->best_makespan = std::min(best_makespan, new_makespan);
        }
        //Data structures used just to track proceedings of the algorithm
        const std::string stats_dump_path;
        std::vector< timeperiod::timeunit > best_makespan_by_iter;
        std::vector< std::vector< timeperiod::timeunit > > makespan_by_iter_by_fs;
        //I then need a data structure to track down best action parameters by iter
        std::vector< std::vector< float > > best_Pmpi_by_iter_by_state;
        std::vector< std::vector< int > > best_nrp_by_iter_by_state;
        std::vector< std::vector< int > > best_nswp_by_iter_by_state;
        std::vector< std::vector< int > > best_nmc_by_iter_by_state;
        std::vector< std::vector< float > > best_Phyb_by_iter_by_state;
        //Method to send scout bees, thus pruning non improving solutions
        void scout();
        //Method causing an iteration to take place
        void iterate();
        //Method to register on inner data structures the tracking of the search
        void registerIterationStats();
        void saveStats() const;
        void saveFitnessStats() const;
        void saveBestActionParameters() const;
    };

    //I shall then implement some support functions for my problem

    /*  this function shall get the action out of a probability */
    int getActionFromProbEGreedy(const int n_actions, const int best_action, float prob, const float eps);

}

#endif //SLABC_H
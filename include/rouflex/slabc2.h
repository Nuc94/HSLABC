#ifndef SLABC2_H
#define SLABC2_H

#include <string>
#include <vector>
#include <random>
#include <memory>
#include <fstream>
#include <thread>
#include <atomic>

#include <math.h>

#include "../../include/rouflex/instance.h"
#include "../../include/rouflex/encoding.h"
#include "../../include/rouflex/decoder.h"

namespace rouflex {

    class EpsActionProvider {
    public:
        //I just need a dump default constructor
        EpsActionProvider() : learning_rate{0.75}, discount_factor{0.25}, eps{0.3} {}
        //This is the real constructor which shall be invoked
        EpsActionProvider(   const int n_states, const int n_actions, const float eps,
                            const double learning_rate, const double discount_factor);
        int getEGreedyAction(const int state, std::default_random_engine & reng) const;
        inline int getBestActionAtState(const int state) const {
            return this->best_action_by_state[ state ];
        }
        void updateByRewardQL(const int state, const int action, const int new_state, const double reward);
        void updateByRewardSARSA(const int state, const int action, const int new_state, const int new_action, const double reward);
        inline int getNStates() const {
            return this->Qsa_matrix.size();
        }
        inline bool hasMultipleActions() const {
            return n_actions > 1;
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

    class SLABC2 {
    public:
        SLABC2(
            const Instance & instance_inp,
            const int nIter_inp,
            const int nEP_inp,
            const int nOP_inp,
            const int nSC_inp,
            const int limit_up_inp,
            const int limit_low_inp,
            const float Phyb_init_inp,
            const float eps_inp,
            const double learning_rate_inp,
            const double discount_factor_inp,
            const double no_improvement_reward_inp,
            std::default_random_engine & rengine_inp,
            const std::vector< double > & state_score_thresholds_inp,
            const std::vector< float > & Pmpi_vector_inp,
            const std::vector< int > & nrp_vector_inp,
            const std::vector< float > & Pswap_vector_inp,
            const std::vector< float > & Phyb_swap_vector_inp,
            const std::vector< float > & Phyb_switch_vector_inp,
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
        const int nIter;
        const int nOP; //Number of Onlooker Bees
        const int nSC; //Number of Scout Bees
        const int limit_up;
        const int limit_low;
        const double limit_rate;
        const float Phyb_init;
        const float eps;
        const double learning_rate;
        const double discount_factor;
        const double no_improvement_reward;
        std::default_random_engine & rengine;
        Decoder dec;
        inline int getNStates() const { return this->fitness_state_score_thresholds.size() + 1; }
        std::vector< std::unique_ptr<Encoding> > food_sources; //This vector shall contain food sources encodings
        std::vector< int > no_improvement_cycles;
        void exploreFS(const int fs_index);
        void scoutFS(const int fs_index);
        int getLimitByStateScore(const double state_score) const;
        int binaryTournamentFoodSource() const;
        timeperiod::timeunit best_makespan;
        std::vector< double > fitness_state_score_thresholds;
        double getFitnessStateScore(const timeperiod::timeunit fitness);
        int getFitnessState(const timeperiod::timeunit fitness);
        inline double getReward(const timeperiod::timeunit original_fitness, const timeperiod::timeunit new_fitness) {
            if(new_fitness <= original_fitness) return original_fitness - new_fitness;
            return no_improvement_reward;
        }
        EpsActionProvider PmpiActionProvider;
        EpsActionProvider NrpActionProvider;
        EpsActionProvider ProbSwapActionProvider;
        EpsActionProvider SwaPhybActionProvider;
        EpsActionProvider SwitchPhybActionProvider;
        std::vector< float > Pmpi_by_action;
        std::vector< int > nrp_by_action;
        std::vector< float > Pswap_by_action;
        std::vector< float > swap_Phyb_by_action;
        std::vector< float > switch_Phyb_by_action;
        int getPmpiAction(const int state) const;
        int getNrpAction(const int state) const;
        int getSwaProbAction(const int state) const;
        int getSwaPhybAction(const int state) const;
        int getSwitchPhybAction(const int state) const;
        void buildPmpiActions( const std::vector< float > & Pmpi_vector_inp );
        void buildNrpActions( const std::vector< int > & nrp_vector_inp );
        void buildPswapActions( const std::vector< float > & Pswap_vector_inp );
        void buildSwapPhybActions( const std::vector< float > & Phyb_swap_vector_inp );
        void buildSwitchPhybActions( const std::vector< float > & Phyb_switch_vector_inp );
        void rewardActions(const int state, const int new_state, const double reward, const int Pmpi_action, const int nrp_action, const int Pswap_action, const int swap_Phyb_action, const int switch_Phyb_action);
        void rewardAction(const int state, const int new_state, const double reward, const int action, EpsActionProvider & action_provider);
        inline float getPmpiFromPmpiAction(const int Pmpi_action) const {
            assert( this->Pmpi_by_action.size() > Pmpi_action );
            return this->Pmpi_by_action[ Pmpi_action ];
        }
        inline int getNrpFromAction(const int nrp_action) const {
            assert( this->nrp_by_action.size() > nrp_action );
            return this->nrp_by_action[ nrp_action ];
        }
        inline float getPswapFromAction(const int Pswap_action) const {
            assert( this->Pswap_by_action.size() > Pswap_action );
            return this->Pswap_by_action[ Pswap_action ];
        }
        inline float getSwapPhybFromAction(const int swapPhyb_action) const {
            assert( this->swap_Phyb_by_action.size() > swapPhyb_action );
            return this->swap_Phyb_by_action[ swapPhyb_action ];
        }
        inline float getSwitchPhybFromAction(const int switchPhyb_action) const {
            assert( this->switch_Phyb_by_action.size() > switchPhyb_action );
            return this->switch_Phyb_by_action[ switchPhyb_action ];
        }
        void updateFoodSource(const int fs_index, std::unique_ptr<Encoding>& candidate_food_source);
        inline void updateBestMakespan(const timeperiod::timeunit new_makespan) {
            this->best_makespan = std::min(best_makespan, new_makespan);
        }

        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        //LEGACY PRIVATE METHODS
        
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
        //and now a pointer to the best encoding
        inline void updateBestEncoding(const std::unique_ptr<Encoding>& candidate_best_encoding) {
            if(! this->best_encoding) this->best_encoding = std::unique_ptr<Encoding>( new Encoding(*candidate_best_encoding) );
            else if(candidate_best_encoding->getMakespan() < this->best_encoding->getMakespan()) { this->best_encoding = std::unique_ptr<Encoding>( new Encoding(*candidate_best_encoding) );
            this->saveBestEncoding();
            }  
        }
        void saveBestEncoding() const;
        std::unique_ptr<Encoding> best_encoding;
    };

    //I shall then implement some support functions for my problem

    /*  this function shall get the action out of a probability */
    int getActionFromProbEGreedy(const int n_actions, const int best_action, float prob, const float eps);

}

#endif //SLABC2_H
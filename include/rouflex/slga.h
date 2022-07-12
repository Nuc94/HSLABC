/*
    A Self-Learning Genetic Algorithm shall hereby be implemented
*/

#ifndef SLGA_H
#define SLGA_H

#include <vector>
#include <random>
#include <memory>

#include "../../include/rouflex/instance.h"
#include "../../include/rouflex/encoding.h"
#include "../../include/rouflex/decoder.h"
#include "../../include/rouflex/slabc2.h"

namespace rouflex {

    class SLGA {
    public:
        SLGA(
            const Instance & instance_inp,
            const unsigned int nIter_inp,
            const unsigned int pop_size_inp,
            const float eps_inp,
            const double learning_rate_inp,
            const double discount_factor_inp,
            const double no_improvement_reward_inp,
            const double wf_inp,
            const double wd_inp,
            const double wm_inp,
            std::default_random_engine & rengine_inp,
            const std::vector<double> & state_score_thresholds_inp,
            const std::vector<double> & pc_by_action_inp,
            const std::vector<double> & pm_by_action_inp,
            const std::string & stats_dump_path_inp
        );
        inline timeperiod::timeunit getBestMakespan() const {
            return this->best_solution->getMakespan();
        }
        timeperiod::timeunit solve();
    private:
        const Instance & instance;
        const unsigned int nIter;
        const unsigned int pop_size;
        const float eps;
        const double learning_rate;
        const double discount_factor;
        const double no_improvement_reward;
        const double wf;
        const double wd;
        const double wm;
        const int sarsa_update_threshold;
        std::default_random_engine & rengine;
        const std::string & stats_dump_path;
        Decoder dec;
        std::unique_ptr<Encoding> best_solution;
        std::unique_ptr< std::vector< std::unique_ptr<Encoding> > > pop;
        std::vector<timeperiod::timeunit> best_sol_tracking;
        //method to record best_fitness
        inline void trackBestSolutionFound() {
            this->best_sol_tracking.push_back( this->getBestMakespan() );
        }
        //method for initialization of population
        void initPop();
        //method to initialize parameters of starting generation for state stats
        void initStateStatsParams();
        //method to decode the population
        void decodePop();
        /*method to seek best solution out of existing population, and
        also stats for determining state*/
        void checkBestSolAndStats();
        void savePrevStats();
        //this method shall calculate the rewards
        void calcRewards();
        //iteration method
        void iterate(const int n_iter);
        //this method shall evelce the population
        void evolvePop(const double pc, const double pm);
        //method to obtain a tournament selection position in the pop vector
        int getBinTourSelectionPos();
        //method to get the counter of the elements which will survive next gen
        std::pair< std::vector<int>, std::vector<int> > getBinTourSelection();
        //method to update the population after a selection
        void buildNewPop();
        //initial state stats parameters
        int first_pop_fit_sum;
        int first_pop_fit_var;
        int first_pop_fit_max;
        int pop_fit_sum;
        int pop_fit_var;
        int pop_fit_max;
        int prev_pop_fit_sum;
        int prev_pop_fit_max;
        double reward_cross;
        double reward_mut;
        double prev_reward_cross;
        double prev_reward_mut;
        inline double getFScore() const {
            return static_cast<double>(this->pop_fit_sum) / static_cast<double>(this->first_pop_fit_sum);
        }
        inline double getDScore() const {
            return static_cast<double>(this->pop_fit_var) / static_cast<double>(this->first_pop_fit_var);
        }
        inline double getMScore() const {
            return static_cast<double>(this->pop_fit_max) / static_cast<double>(this->first_pop_fit_max);
        }
        inline double getStateScore() const {
            double state_score = 0;
            state_score += this->wf * this->getFScore();
            state_score += this->wd * this->getDScore();
            state_score += this->wm * this->getMScore();
            return state_score;
        }
        int state;
        int prev_state;
        void calcState();
        inline void savePrevState() {
            this->prev_state = this->state;
        }
        std::vector<double> state_score_thresholds;
        std::vector<double> pc_by_action;
        std::vector<double> pm_by_action;
        inline int getNStates() const { return this->state_score_thresholds.size() + 1; }
        EpsActionProvider PcActionProvider;
        EpsActionProvider PmActionProvider;
        void initActionProviders();
        inline bool isUpdateSARSA(const int n_iter) { return n_iter < this->sarsa_update_threshold; }
        int pc_action;
        int pm_action;
        int prev_pc_action;
        int prev_pm_action;
        void updateActionProviders(const int n_iter);
        inline void savePrevActions() {
            this->prev_pc_action = this->pc_action;
            this->prev_pm_action = this->pm_action;
        }
        void calcActions();
        inline double getPc() const {
            return this->pc_by_action[this->pc_action];
        }
        inline double getPm() const {
            return this->pm_by_action[this->pm_action];
        }
        //method to register data
        void saveFitnessStats() const;
    };

    /*
    Here is the class for parallel execution
    */

    class ExecSLGA {
    public:
        ExecSLGA(   const Instance & instance_inp,
                    const std::string stats_sink_path_inp,
                    const int seed_inp);
        void execute() const;
    private:
        const Instance & instance;
        const std::string stats_sink_path;
        const int seed;
    };

    void slga_parallel_exec(std::vector< std::unique_ptr<ExecSLGA> > & SLGAs, std::atomic<int> & global_read_point);

};

#endif //SLGA_H
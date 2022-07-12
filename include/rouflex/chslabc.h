#ifndef CHSLABC_H
#define CHSLABC_H

#include <vector>
#include <random>
#include <memory>

#include "../../include/rouflex/instance.h"
#include "../../include/rouflex/encoding.h"
#include "../../include/rouflex/decoder.h"
#include "../../include/rouflex/slabc2.h"

namespace rouflex {

    /*
        A dumb function in order to easily obtain the inverse of a number
    */

    inline double inv(double num) {
        return 1 / num;
    }

    /*
        This class shall implement a self-learning artificial bee colony
    */

    class CHSLABC {
    public:
        CHSLABC(
            const Instance & instance_inp,
            const unsigned int nIter_inp,
            const unsigned int pop_size_inp,
            const unsigned int limit_inp,
            const float eps_inp,
            const double learning_rate_inp,
            const double discount_factor_inp,
            const double no_improvement_reward_inp,
            const double wa_inp,
            const double wv_inp,
            const double wm_inp,
            const int rengine_seed,
            const std::vector<double> & state_score_thresholds_inp,
            const std::vector<double> & evo_size_actions_inp,
            const std::string & stats_dump_path_inp
        );
        timeperiod::timeunit solve();

    private:
        const Instance & instance;
        const unsigned int nIter;
        const unsigned int pop_size;
        const unsigned int limit;
        const float eps;
        const double learning_rate;
        const double discount_factor;
        const double no_improvement_reward;
        const double wa;
        const double wv;
        const double wm;
        const int rengine_seed;
        std::default_random_engine rengine;
        const std::vector<double> state_score_thresholds;
        const std::vector<double> evo_size_actions;
        const std::string & stats_dump_path;
        void init();
        void initPop();
        void initFirstStateStats();
        Decoder dec;
        EpsActionProvider action_provider;
        std::vector< std::unique_ptr<Encoding> > food_sources;
        std::unique_ptr<Encoding> best_solution;
        //attributes aimed at handling actions and updates
        int past_state;
        int past_action;
        int state;
        int action;
        int first_best_makespan_of_gen;
        int past_best_makespan_of_gen;
        int best_makespan_of_gen;
        //parameters for state calculation
        double first_sum_of_fitness;
        double first_fitness_var;
        double first_best_fitness_of_gen;
        //methods to get state stats
        double getFitnessSum();
        double getFitnessVar(const double fitness_sum);
        int getBestMakespanOfGen();
        //method to handle iterations
        void iterate();
        void calcBestMakespanOfGen();
        void calcStateFromFoodSources();
        void calcAction();
        void updateRL();
        void savePastStatsRL();
        double getReward();
    };

}

#endif //CHSLABC_H
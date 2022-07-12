#include <iostream>
#include <string>
#include <random>

#include "../include/rouflex/instance.h"
#include "../include/rouflex/slga.h"

int main() {

    const rouflex::Instance instance_inp("/home/nuc/Programmazione/C++/RouFlex/data/B3/Mk10.json");
    const unsigned int nIter_inp = 1000;
    const unsigned int pop_size_inp = 1000;
    const float eps_inp = 0.85;
    const double learning_rate_inp = 0.75;
    const double discount_factor_inp = 0.2;
    const double no_improvement_reward_inp = 0;
    const double wf_inp = 0.35;
    const double wd_inp = 0.35;
    const double wm_inp = 0.3;
    std::default_random_engine rengine_inp(6);
    std::vector<double> state_score_thresholds_inp;
    std::vector<double> pc_by_action_inp;
    std::vector<double> pm_by_action_inp;
    const std::string stats_dump_path_inp = "boh for now";

    for(double s = 0.05; s <= 1.0; s += 0.05) {
        state_score_thresholds_inp.push_back(s);
    }
    for(double pc = 0.4; pc <= 0.9; pc += 0.05) {
        pc_by_action_inp.push_back(pc);
    }
    for(double pm = 0.01; pm <= 0.21; pm += 0.02) {
        pm_by_action_inp.push_back(pm);
    }

    rouflex::SLGA slga(
        instance_inp,
        nIter_inp,
        pop_size_inp,
        eps_inp,
        learning_rate_inp,
        discount_factor_inp,
        no_improvement_reward_inp,
        wf_inp,
        wd_inp,
        wm_inp,
        rengine_inp,
        state_score_thresholds_inp,
        pc_by_action_inp,
        pm_by_action_inp,
        stats_dump_path_inp
    );

    slga.solve();

    return 0;

}
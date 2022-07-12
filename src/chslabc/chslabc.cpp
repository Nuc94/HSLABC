#include "../../include/rouflex/chslabc.h"

rouflex::CHSLABC::CHSLABC(
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
    const int rengine_seed_inp,
    const std::vector<double> & state_score_thresholds_inp,
    const std::vector<double> & evo_size_actions_inp,
    const std::string & stats_dump_path_inp
    ) : instance{instance_inp}, nIter{nIter_inp}, pop_size{pop_size_inp},
        limit{limit_inp}, eps{eps_inp}, learning_rate{learning_rate_inp},
        discount_factor{discount_factor_inp}, no_improvement_reward{no_improvement_reward_inp},
        wa{wa_inp}, wv{wv_inp}, wm{wm_inp}, rengine_seed{rengine_seed_inp},
        state_score_thresholds{state_score_thresholds_inp},
        evo_size_actions{evo_size_actions_inp},
        stats_dump_path{stats_dump_path_inp}, best_solution{nullptr}
    {}

timeperiod::timeunit rouflex::CHSLABC::solve() {
    this->init();
    for(int iter = 0; iter < this->nIter; ++iter) {
        this->iterate();
    }
}

/*
    This method shall be invoked at the beginning of solve to load up stuff
    for algorithm execution
*/
void rouflex::CHSLABC::init() {
    this->rengine = std::default_random_engine(this->rengine_seed);
    this->dec = Decoder();
    this->action_provider = EpsActionProvider(this->state_score_thresholds.size(), this->evo_size_actions.size(), this->eps, this->learning_rate, this->discount_factor);
    this->initPop();
}

void rouflex::CHSLABC::initPop() {
    this->food_sources.reserve(this->pop_size);
    for(int i = 0; i < this->pop_size; ++i) {
        this->food_sources.emplace_back( std::make_unique<Encoding>(this->instance, this->rengine, true, 0) );
        //HERE DECODING SHALL BE DONE
    }
    this->initFirstStateStats();
}

void rouflex::CHSLABC::initFirstStateStats() {
    this->first_sum_of_fitness = this->getFitnessSum();
    this->first_fitness_var = this->getFitnessVar(this->first_sum_of_fitness);
    this->first_best_fitness_of_gen = this->getBestMakespanOfGen();
}

double rouflex::CHSLABC::getFitnessSum() {
    double result = 0;
    for(const auto & fs : this->food_sources) {
        result += inv(fs->getMakespan());
    }
    return result;
}

double rouflex::CHSLABC::getFitnessVar(const double fitness_sum) {
    const double fitness_avg = fitness_sum / this->pop_size;
    double result = 0;
    for(const auto & fs : this->food_sources) {
        result += std::abs( inv(fs->getMakespan()) - fitness_avg );
    }
    return result;
}

int rouflex::CHSLABC::getBestMakespanOfGen() {
    int result = timeperiod::INFINITE_TIME;
    for(int i = 0; i < food_sources.size(); ++i) {
        result = std::min(this->food_sources[i]->getMakespan(), result);
    }
    return result;
}

void rouflex::CHSLABC::iterate() {
    this->calcStateFromFoodSources();
    this->calcAction();
}

void rouflex::CHSLABC::calcBestMakespanOfGen() {
    this->best_makespan_of_gen = this->getBestMakespanOfGen();
}

void rouflex::CHSLABC::calcStateFromFoodSources() {
    double score, fa, fv, fm, fitness_sum;
    fitness_sum = this->getFitnessSum();
    fa = fitness_sum / this->first_sum_of_fitness;
    fv = this->getFitnessVar(fitness_sum) / this->first_fitness_var;
    fm = inv(this->best_makespan_of_gen) / this->first_best_fitness_of_gen;
    score = this->wa * fa + this->wv * fv + this->wm * fm;
    for(int s = 0; s < this->state_score_thresholds.size(); ++s) {
        if( this->state_score_thresholds[s] >= score ) {
            this->state = s;
            return;
        }
    }
    this->state = this->state_score_thresholds.size();
}

void rouflex::CHSLABC::calcAction() {
    this->action = this->action_provider.getEGreedyAction(this->state, this->rengine);
}

void rouflex::CHSLABC::updateRL() {
    this->action_provider.updateByRewardQL(this->past_state, this->past_action, this->state, this->getReward());
}

void rouflex::CHSLABC::savePastStatsRL() {
    this->past_state = this->state;
    this->past_action = this->action;
    this->past_best_makespan_of_gen = this->best_makespan_of_gen;
}

double rouflex::CHSLABC::getReward() {
    return ( inv(this->best_makespan_of_gen) - inv(this->past_best_makespan_of_gen) ) / this->past_best_makespan_of_gen;
}
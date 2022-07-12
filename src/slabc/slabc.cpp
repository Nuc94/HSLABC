#include "../../include/rouflex/slabc.h"

rouflex::RLActionProvider::RLActionProvider(const int n_states, const int n_actions,
        const float eps, const double learning_rate, const double discount_factor)
 : n_actions{n_actions}, eps{eps}, learning_rate{learning_rate}, discount_factor{discount_factor} {
    this->Qsa_matrix = std::vector< std::vector<double> >( n_states, std::vector<double>(this->n_actions, 0) );
    assert(this->Qsa_matrix.size() == n_states );
    for(const auto & action_values : this->Qsa_matrix) {
        assert( action_values.size() == n_actions );
        for(const double av : action_values) assert(av == 0);
    }
    this->best_action_by_state = std::vector<int>(n_states, 0);
    assert(this->best_action_by_state.size() == this->Qsa_matrix.size());
}

int rouflex::RLActionProvider::getEGreedyAction(const int state, std::default_random_engine & reng) const {
    assert( this->Qsa_matrix.size() == this->best_action_by_state.size() );
    assert( this->best_action_by_state.size() > state );
    const int best_action = this->best_action_by_state[state];
    std::uniform_real_distribution<double> unif(0, 1);
    float prob = unif(reng);
    return getActionFromProbEGreedy(this->n_actions, best_action, prob, this->eps);
}

void rouflex::RLActionProvider::updateByRewardQL(const int state, const int action, const int new_state, const double reward) {
    const int next_supposed_action = this->best_action_by_state[new_state];
    this->Qsa_matrix[state][action] = (1 - this->learning_rate) * this->Qsa_matrix[state][action] + this->learning_rate * ( reward + this->discount_factor * this->Qsa_matrix[new_state][next_supposed_action]);
    this->updateBestAction(state, action);
}

void rouflex::RLActionProvider::updateBestAction(const int state, const int action) {
    const int past_best_action = this->best_action_by_state[state];
    if( this->Qsa_matrix[state][action] > this->Qsa_matrix[state][past_best_action] ) this->best_action_by_state[state] = action;
}

rouflex::SLABC::SLABC(
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
        ) : instance{instance_inp}, nIter{nIter_inp}, nOP{nOP_inp},
            limit{limit_inp}, Phyb_init{Phyb_init_inp}, eps{eps_inp},
            learning_rate{learning_rate_inp}, discount_factor{discount_factor_inp},
            no_improvement_reward{-1 * std::abs(no_improvement_reward_inp)},
            rengine{rengine_inp}, best_makespan{timeperiod::INFINITE_TIME},
            fitness_state_score_thresholds{state_score_thresholds_inp},
            stats_dump_path{stats_dump_path_inp} {
    this->buildPmpiActions(Pmpi_vector_inp);
    this->buildCrossActions(nrp_vector_inp);
    this->buildMutActions(nswp_vector_inp, nmc_vector_inp, Phyb_vector_inp);
    //I shall then initialize the food sources
    this->no_improvement_cycles = std::vector<int>(nEP_inp, 0);
    this->food_sources.reserve(nEP_inp);
    for(unsigned int i = 0; i < nEP_inp; ++i) {
        this->food_sources.emplace_back( std::make_unique<Encoding>(this->instance, this->rengine, true, this->Phyb_init) );
        this->dec.decode(*this->food_sources.back(), true);
        this->updateBestMakespan(this->food_sources.back()->getMakespan());
    }
    assert(this->food_sources.size() == nEP_inp);
}

void rouflex::SLABC::exploreFoodSource(const int fs_index) {
    assert( this->food_sources.size() > fs_index );
    std::unique_ptr<Encoding> candidate_food_source = std::unique_ptr<Encoding>( new Encoding(*(this->food_sources[fs_index])) );
    int cross_food_source; //this one shall be the foodsource on which to perform pseudo crossover neighborhood exploration
    const timeperiod::timeunit original_fitness = this->food_sources[fs_index]->getMakespan();
    const int original_state = this->getFitnessState(original_fitness);
    //if(original_state != 0) std::cout << "State: " << original_state << std::endl;
    int next_state;
    //Since Pmpi action decision needs to be taken I shall 
    const int Pmpi_action = this->getPmpiAction(original_state);
    //Subordinate action choices are supposed to start as -1, so that when they are chosen I know
    //that a learning step was taken for that action field
    int cross_action = -1;
    int mut_action = -1;
    timeperiod::timeunit new_fitness;
    const float Pmpi = this->getPmpiFromPmpiAction(Pmpi_action);
    int nrp, nswp, nmc;
    float Phyb;
    double reward;
    std::bernoulli_distribution bd( Pmpi );
    //Next I shall evolve my candidate food source according to the random result
    if ( bd(this->rengine) ) {
        //In this case I shall perform the pseudo crossover
        //At first I select a food source for crossover
        cross_food_source = this->rengine() % this->getNFoodSources();
        while( cross_food_source == fs_index ) cross_food_source = this->rengine() % this->getNFoodSources();
        //after having selected a food source from crossover, I shall select crossover parameters from action
        cross_action = this->getCrossAction(original_state);
        nrp = this->getNrpFromCrossAction(cross_action);
        //after having selected a crossover
        candidate_food_source->permuteByOther( *(this->food_sources[ cross_food_source ]), nrp, this->rengine );
    } else {
        //otherwise I shall perform mutation
        mut_action = this->getMutAction(original_state);
        nswp = this->getNswpFromMutAction(mut_action);
        nmc = this->getNswpFromMutAction(mut_action);
        Phyb = this->getNswpFromMutAction(mut_action);
        //std::cout << mut_action << " " << nswp << " " << nmc << " " << Phyb << std::endl;
        candidate_food_source->perturbation( nswp, nmc, Phyb, this->rengine);
    }
    this->dec.decode(*candidate_food_source, true);
    //this->food_sources[fs_index]->logEncoding();
    //std::cout << "\n\n";
    //candidate_food_source->logEncoding();
    //std::cout << "\n\n\n\n\n\n";
    assert(candidate_food_source->getMakespan() > 0);
    //NEXT I SHALL TAKE REWARD, UPDATE Q VALUES AND SOLUTUIONS
    reward = this->getReward(original_fitness, candidate_food_source->getMakespan());
    this->updateFoodSource(fs_index, candidate_food_source);
    next_state = this->getFitnessState(this->food_sources[fs_index]->getMakespan());
    //I SHALL NOW UPDATE Q VALUES BY REWARD
    PmpiActionProvider.updateByRewardQL(original_state, Pmpi_action, next_state, reward);
    if(cross_action != -1) {
        CrossActionProvider.updateByRewardQL(original_state, cross_action, next_state, reward);
    }
    if(mut_action != -1) {
        MutActionProvider.updateByRewardQL(original_state, mut_action, next_state, reward);
    }
}

int rouflex::SLABC::binaryTournamentFoodSource() const {
    int fs1, fs2;
    fs1 = this->rengine() % this->getNFoodSources();
    fs2 = this->rengine() % this->getNFoodSources();
    while( fs1 == fs2 ) fs2 = this->rengine() % this->getNFoodSources();
    if(this->food_sources[fs1]->getMakespan() > this->food_sources[fs2]->getMakespan() ) return fs2;
    else return fs1;
}

double rouflex::SLABC::getFitnessStateScore(const timeperiod::timeunit fitness) {
    //std::cout << "Best Makespan: " << this->getBestMakespan() << std::endl;
    //std::cout << "Difference: " << std::abs( fitness - this->getBestMakespan() ) << std::endl;
    return static_cast<double>( std::abs( fitness - this->getBestMakespan() ) ) / static_cast<double>( fitness );
}

int rouflex::SLABC::getFitnessState(const timeperiod::timeunit fitness) {
    //std::cout << "Fitness: " << fitness << std::endl;
    double score = this->getFitnessStateScore(fitness);
    //std::cout << "Score: " << score << std::endl;
    for(int state = 0; state < this->fitness_state_score_thresholds.size(); ++state) {
        if( this->fitness_state_score_thresholds[state] >= score ) return state;
    }
    return this->fitness_state_score_thresholds.size();
}

void rouflex::SLABC::buildPmpiActions(const std::vector< float > & Pmpi_vector_inp) {
    this->Pmpi_by_action = Pmpi_vector_inp;
    this->PmpiActionProvider = RLActionProvider(this->getNStates(), this->Pmpi_by_action.size(), this->eps, this->learning_rate, this->discount_factor);
}

void rouflex::SLABC::buildCrossActions(const std::vector< int > & nrp_vector_inp) {
    this->nrp_by_action = nrp_vector_inp;
    this->CrossActionProvider = RLActionProvider(this->getNStates(), this->nrp_by_action.size(), this->eps, this->learning_rate, this->discount_factor);
}

void rouflex::SLABC::buildMutActions(   const std::vector< int > & nswp_vector_inp,
                                const std::vector< int > & nmc_vector_inp,
                                const std::vector< float > & Phyb_vector_inp) {
    this->nswp_by_action.reserve( nswp_vector_inp.size() * nmc_vector_inp.size() * Phyb_vector_inp.size() );
    this->nmc_by_action.reserve( nswp_vector_inp.size() * nmc_vector_inp.size() * Phyb_vector_inp.size() );
    this->Phyb_by_action.reserve( nswp_vector_inp.size() * nmc_vector_inp.size() * Phyb_vector_inp.size() );
    for(const int nswp : nswp_vector_inp) {
        for(const int nmc : nmc_vector_inp) {
            for(const float Phyb : Phyb_vector_inp) {
                this->nswp_by_action.push_back(nswp);
                this->nmc_by_action.push_back(nmc);
                this->Phyb_by_action.push_back(Phyb);
            }
        }
    }
    assert(this->nswp_by_action.size() == this->nmc_by_action.size());
    assert(this->nswp_by_action.size() == this->Phyb_by_action.size());
    this->MutActionProvider = RLActionProvider(this->getNStates(), this->nswp_by_action.size(), this->eps, this->learning_rate, this->discount_factor);
}

int rouflex::SLABC::getPmpiAction(const int state) const {
    return this->PmpiActionProvider.getEGreedyAction(state, this->rengine);
}

int rouflex::SLABC::getCrossAction(const int state) const {
    return this->CrossActionProvider.getEGreedyAction(state, this->rengine);
}

int rouflex::SLABC::getMutAction(const int state) const {
    return this->MutActionProvider.getEGreedyAction(state, this->rengine);
}


void rouflex::SLABC::updateFoodSource(const int fs_index, std::unique_ptr<Encoding>& candidate_food_source) {
    assert(fs_index < this->food_sources.size());
    auto original_fitness = this->food_sources[fs_index]->getMakespan();
    auto proposed_fitness = candidate_food_source->getMakespan();
    //std::cout << "Original fitness: " << this->food_sources[fs_index]->getMakespan() << std::endl;
    //std::cout << "New fitness: " << candidate_food_source->getMakespan() << std::endl;
    if(candidate_food_source->getMakespan() < this->food_sources[fs_index]->getMakespan()) {
        //In case I obtain a minor makespan, i.e. the fitness, I shall substitute the food source
        /*std::cout << "Possible improvement happened" << std::endl;
        std::cout << "Old makespan: " << this->food_sources[fs_index]->getMakespan() << std::endl;
        std::cout << "Candidate makespan: " << candidate_food_source->getMakespan() << std::endl;*/

        this->food_sources[fs_index] = std::move(candidate_food_source);
        //std::cout << "After substitution: " << this->food_sources[fs_index]->getMakespan() << std::endl;
        this->no_improvement_cycles[fs_index] = -1;
        this->updateBestMakespan(this->food_sources[fs_index]->getMakespan());
    }
    auto new_fitness = this->food_sources[fs_index]->getMakespan();
    assert(new_fitness <= original_fitness);
    //std::cout << "After evaluation: " << this->food_sources[fs_index]->getMakespan() << std::endl;
}

void rouflex::SLABC::scout() {
    assert(this->food_sources.size() == this->no_improvement_cycles.size());
    for(int i = 0; i < this->no_improvement_cycles.size(); i++) {
        //std::cout << "Limit: " << this->limit << std::endl;
        if( this->no_improvement_cycles[i] >= this->limit ) {
            //std::cout << "No improvement cycles: " << this->no_improvement_cycles[i] << std::endl;
            this->food_sources[i] = std::unique_ptr<Encoding>( new Encoding(this->instance, this->rengine, true, this->Phyb_init) );
            this->dec.decode(*this->food_sources[i], true);
            this->updateBestMakespan(this->food_sources[i]->getMakespan());
            this->no_improvement_cycles[i] = -1;
        }
        ++this->no_improvement_cycles[i];
    }
}

void rouflex::SLABC::iterate() {
    int fs_index;
    //I start by employng bees to explore foodsources
    for(fs_index = 0; fs_index < this->food_sources.size(); ++fs_index) this->exploreFoodSource(fs_index);
    //I then use onlooker bees to explore food sources
    for(int i = 0; i < this->nOP; ++i) {
        fs_index = this->binaryTournamentFoodSource();
        this->exploreFoodSource(fs_index);
    }
    //I shall the send scout bees to prune non-improving solutions
    this->scout();
    std::cout << "Best makespan: " << this->getBestMakespan() << std::endl;
    this->registerIterationStats();
}

void rouflex::SLABC::registerIterationStats() {
    int bestPmpi_action_at_state, best_crossaction_at_state, best_mutaction_at_state;
    //At first I register whatever necessary for the food sources fitness tracking
    this->best_makespan_by_iter.push_back( this->getBestMakespan() );
    this->makespan_by_iter_by_fs.emplace_back( std::vector<timeperiod::timeunit>(0) );
    this->makespan_by_iter_by_fs.back().reserve( this->getNFoodSources() );
    for(int fs = 0; fs < this->getNFoodSources(); ++fs) this->makespan_by_iter_by_fs.back().push_back( this->food_sources[fs]->getMakespan() );
    //Then I shall register what's necessary to track the best action for every state
    this->best_Pmpi_by_iter_by_state.emplace_back( std::vector<float>() );
    this->best_nrp_by_iter_by_state.emplace_back( std::vector<int>() );
    this->best_nswp_by_iter_by_state.emplace_back( std::vector<int>() );
    this->best_nmc_by_iter_by_state.emplace_back( std::vector<int>() );
    this->best_Phyb_by_iter_by_state.emplace_back( std::vector<float>() );
    //I reserve enough space for states
    this->best_Pmpi_by_iter_by_state.back().reserve( this->getNStates() );
    this->best_nrp_by_iter_by_state.back().reserve( this->getNStates() );
    this->best_nswp_by_iter_by_state.back().reserve( this->getNStates() );
    this->best_nmc_by_iter_by_state.back().reserve( this->getNStates() );
    this->best_Phyb_by_iter_by_state.back().reserve( this->getNStates() );
    for(int state = 0; state < this->getNStates(); ++state) {
        bestPmpi_action_at_state = this->PmpiActionProvider.getBestActionAtState(state);
        this->best_Pmpi_by_iter_by_state.back().push_back( this->Pmpi_by_action[ bestPmpi_action_at_state ] );
        best_crossaction_at_state = this->CrossActionProvider.getBestActionAtState(state);
        this->best_nrp_by_iter_by_state.back().push_back( this->nrp_by_action[ best_crossaction_at_state ] );
        best_mutaction_at_state = this->MutActionProvider.getBestActionAtState(state);
        this->best_nswp_by_iter_by_state.back().push_back( this->nswp_by_action[best_mutaction_at_state] );
        this->best_nmc_by_iter_by_state.back().push_back( this->nmc_by_action[best_mutaction_at_state] );
        this->best_Phyb_by_iter_by_state.back().push_back( this->Phyb_by_action[best_mutaction_at_state] );
    }
}


void rouflex::SLABC::saveStats() const {
    this->saveFitnessStats();
    this->saveBestActionParameters();
}

void rouflex::SLABC::saveFitnessStats() const {
    const std::string filepath = this->stats_dump_path + "/fitness_stats.csv";
    std::ofstream csv_file;
    csv_file.open(filepath);
    csv_file << "iter";
    csv_file << ",best";
    for(int fs = 0; fs < this->getNFoodSources(); ++fs) csv_file << ",fs" + std::to_string(fs) + "fit";
    assert(this->best_makespan_by_iter.size() == this->makespan_by_iter_by_fs.size());
    for(int iter = 0; iter < this->best_makespan_by_iter.size(); ++iter){
        csv_file << "\n";
        csv_file << std::to_string(iter);
        csv_file << "," + std::to_string(this->best_makespan_by_iter[iter]);
        for(int fs = 0; fs < this->getNFoodSources(); ++fs) csv_file << "," + std::to_string(this->makespan_by_iter_by_fs[iter][fs]);
    }
    csv_file.close();
}

void rouflex::SLABC::saveBestActionParameters() const {
    const std::string filepath = this->stats_dump_path + "/action_stats.csv";
    std::ofstream csv_file;
    csv_file.open(filepath);
    csv_file << "iter";
    for(int state = 0; state < this->getNStates(); ++state) {
        csv_file << ",state" << state << "Pmpi";
        csv_file << ",state" << state << "nrp";
        csv_file << ",state" << state << "nswp";
        csv_file << ",state" << state << "nmc";
        csv_file << ",state" << state << "Phyb";
    }
    for(int iter = 0; iter < this->best_Pmpi_by_iter_by_state.size(); ++iter){
        csv_file << "\n";
        csv_file << std::to_string(iter);
        for(int state = 0; state < this->getNStates(); ++state) {
            csv_file << "," << this->best_Pmpi_by_iter_by_state[iter][state];
            csv_file << "," << this->best_nrp_by_iter_by_state[iter][state];
            csv_file << "," << this->best_nswp_by_iter_by_state[iter][state];
            csv_file << "," << this->best_nmc_by_iter_by_state[iter][state];
            csv_file << "," << this->best_Phyb_by_iter_by_state[iter][state];
        }
    }
    csv_file.close();
}

/*  this function shall get the action out of a probability */
int rouflex::getActionFromProbEGreedy(const int n_actions, const int best_action, float prob, const float eps) {
    if(prob <= eps) return best_action;
    //other wise I shall select among all actions
    prob = (prob - eps) / (1 - eps);
    //std::cout << static_cast<int> (prob * n_actions) % n_actions << std::endl;
    return static_cast<int> (prob * n_actions) % n_actions;
}
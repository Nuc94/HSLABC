#include "../../include/rouflex/sa.h"

rouflex::SA::SA(
    const Instance & instance_inp,
    const double starting_temperature_inp,
    const double min_temperature_inp,
    const double alpha_inp,
    const int max_iterations_inp,
    std::default_random_engine & rengine_inp
) : instance{instance_inp}, starting_temperature{starting_temperature_inp},
    min_temperature{min_temperature_inp}, alpha{alpha_inp}, max_iterations{max_iterations_inp},
    rengine{rengine_inp}, temperature{starting_temperature_inp}, n_iterations{0},
    n_simulations{0}, current_makespan{timeperiod::INFINITE_TIME},
    best_makespan{timeperiod::INFINITE_TIME} {

}

void rouflex::SA::execute() {
    this->initializeRootSolution();
    while( ! this->exitCondition() ) {
        this->iterate();
        //std::cout << this->getBestMakespan() << std::endl;
    }
}

void rouflex::SA::makespanToCsv(const std::string & filepath) const {
    std::string result;
    std::ofstream file(filepath);
    
    result += "starting_temperature,";
    result += "min_temperature,";
    result += "alpha,";
    result += "max_iterations,";
    result += "n_iterations,";
    result += "n_substitutions,";
    result += "n_simulations,";
    result += "Makespan";

    result += "\n";

    result += std::to_string(this->starting_temperature) + ",";
    result += std::to_string(this->min_temperature) + ",";
    result += std::to_string(this->alpha) + ",";
    result += std::to_string(this->max_iterations) + ",";
    result += std::to_string(this->n_iterations) + ",";
    result += std::to_string(this->n_substitutions) + ",";
    result += std::to_string(this->n_simulations) + ",";
    result += std::to_string(this->best_makespan);

    file << result;
    file.close();
}

void rouflex::SA::iterate() {
    timeperiod::timeunit best_neighbor_makespan = timeperiod::INFINITE_TIME;
    int best_neighbor_index = 0;
    std::vector< std::unique_ptr<Encoding> > neighbors = this->root_solution->getSANeighbors( this->rengine );
    //std::cout << neighbors.size() << std::endl;
    for(int i = 0; i < neighbors.size(); ++i) {
        this->simulateNeighbor( neighbors[i] );
        if( neighbors[i]->getMakespan() < best_neighbor_makespan ) {
            best_neighbor_makespan = neighbors[i]->getMakespan();
            best_neighbor_index = i;
        }
    }
    //std::cout << neighbors[ best_neighbor_index ]->getMakespan() << std::endl;
    this->handleNeighbor( neighbors[ best_neighbor_index ] );
    ++this->n_iterations;
}

bool rouflex::SA::exitCondition() const {
    return (this->temperature <= this->min_temperature) || (this->n_iterations >= this->max_iterations);
}

void rouflex::SA::simulateNeighbor( std::unique_ptr<Encoding> & neighbor ) {
    this->dec.decode( *(neighbor), true );
    ++n_simulations;
}

bool rouflex::SA::shallSubstituteSolution( const timeperiod::timeunit neighbor_makespan ) {
    float prob, substitution_prob;
    std::uniform_real_distribution<double> unif(0, 1);
    if(neighbor_makespan <= this->current_makespan) return true;
    prob = unif( this->rengine );
    substitution_prob = std::exp( ( this->current_makespan - neighbor_makespan ) / this->temperature );
    if(prob <= substitution_prob) return true;
    return false;
}

void rouflex::SA::substituteSolution( std::unique_ptr<Encoding> & neighbor ) {
    this->root_solution = std::move(neighbor);
    this->current_makespan = this->root_solution->getMakespan();
    if( this->current_makespan < this->best_makespan ) this->best_makespan = this->current_makespan;
    ++this->n_substitutions;
    this->updateTemperature();
}

void rouflex::SA::handleNeighbor( std::unique_ptr<Encoding> & neighbor ) {
    if( this->shallSubstituteSolution( neighbor->getMakespan() ) ) {
        this->substituteSolution( neighbor );
    }
}

void rouflex::SA::updateTemperature() {
    this->temperature = this->alpha * this->temperature;
}

void rouflex::SA::initializeRootSolution() {
    //I start by initializing the rootsolution randomly
    std::unique_ptr<Encoding> enc_dispatch;
    this->root_solution = std::make_unique<Encoding>(this->instance, this->rengine, true, 1.0);
    this->dec.decode( *(this->root_solution), true );
    this->current_makespan = this->root_solution->getMakespan();
    this->best_makespan = this->root_solution->getMakespan();
    //Now I shall create encodings based on dispatching rules and eventually substitute them to the root
    //Largest Number of Operations
    enc_dispatch = std::make_unique<Encoding>(this->instance, this->rengine, true, 1.0);
    enc_dispatch->dispatchLNO();
    this->dec.decode( *(enc_dispatch), true );
    if (enc_dispatch->getMakespan() < this->best_makespan) {
        this->current_makespan = enc_dispatch->getMakespan();
        this->best_makespan = enc_dispatch->getMakespan();
        this->root_solution = std::move( enc_dispatch );
    }
    //Largest Job Duration
    enc_dispatch = std::make_unique<Encoding>(this->instance, this->rengine, true, 1.0);
    enc_dispatch->dispatchLJD();
    this->dec.decode( *(enc_dispatch), true );
    if (enc_dispatch->getMakespan() < this->best_makespan) {
        this->current_makespan = enc_dispatch->getMakespan();
        this->best_makespan = enc_dispatch->getMakespan();
        this->root_solution = std::move( enc_dispatch );
    }
    //Largest Resources Available
    enc_dispatch = std::make_unique<Encoding>(this->instance, this->rengine, true, 1.0);
    enc_dispatch->dispatchLRA();
    this->dec.decode( *(enc_dispatch), true );
    if (enc_dispatch->getMakespan() < this->best_makespan) {
        this->current_makespan = enc_dispatch->getMakespan();
        this->best_makespan = enc_dispatch->getMakespan();
        this->root_solution = std::move( enc_dispatch );
    }
}
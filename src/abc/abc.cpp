#include "../../include/rouflex/abc.h"

rouflex::ABC::ABC(  const Instance & inp_instance,
                    const unsigned int inp_nIter,
                    const unsigned int inp_nEP,
                    const unsigned int inp_nOP,
                    const unsigned int inp_limit,
                    const float inp_Pbt,
                    const float inp_Pmpi,
                    const unsigned int inp_nrp,
                    const unsigned int inp_nswp,
                    const unsigned int inp_nmc,
                    const float inp_Pfinit,
                    const float inp_Pf,
                    std::default_random_engine & reng) :   instance(inp_instance),
                                    nIter(inp_nIter),
                                    nEP(inp_nEP), nOP(inp_nOP),
                                    limit(inp_limit), Pbt(inp_Pbt),
                                    Pmpi(inp_Pmpi),nrp(inp_nrp),
                                    nswp(inp_nswp),nmc(inp_nmc),
                                    Pfinit(inp_Pfinit),Pf(inp_Pf),
                                    rengine(reng), best_makespan(timeperiod::INFINITE_TIME) {
    this->no_improvement_cycles = std::vector<int>(this->nEP, 0);
    this->food_sources.reserve(this->nEP);
    this->employer_bees.reserve(this->nEP);
    for(int i = 0; i < this->nEP; ++i) {
        this->food_sources.emplace_back( std::make_unique<Encoding>(this->instance, this->rengine, true, this->Pfinit) );
        //if(i == 0) this->food_sources[0]->logEncoding();
        if(!this->food_sources.back()->isResourceAssignmentCorrect()) this->food_sources.back()->logEncoding();
        assert(this->food_sources.back()->isSequencingCorrect());
        assert(this->food_sources.back()->isResourceAssignmentCorrect());
        this->dec.decode(*(this->food_sources[i]), true);
        //if(i == 0) this->food_sources[0]->logEncoding();
        if(!this->food_sources.back()->isResourceAssignmentCorrect()) this->food_sources.back()->logEncoding();
        assert(this->food_sources.back()->isSequencingCorrect());
        assert(this->food_sources.back()->isResourceAssignmentCorrect());
        this->updateMakespan( this->food_sources[i]->getMakespan() );
    }
    this->onlooker_bees.reserve(this->nOP);
    this->onlooker_origin = std::vector<int>(this->nOP, 0);
    this->makespan_at_gen.reserve(this->nIter);
}

timeperiod::timeunit rouflex::ABC::solve() {
    for(int i = 0; i < this->nIter; ++i) {
        //std::cout << "Iteration" << i << std::endl;
        //std::cout << this->getBestMakespan() << std::endl;
        this->iterate();
    }
    return this->getBestMakespan();
}

void rouflex::ABC::iterate() {
    int rand_food_index;
    this->employer_bees.clear();
    for(int i = 0; i < this->nEP; ++i) {
        //std::cout << i << std::endl;
        rand_food_index = this->rengine() % this->nEP;
        //std::cout << rand_food_index << std::endl;
        this->employer_bees.emplace_back( std::make_unique<Encoding>(*this->food_sources[i]) );
        if(!this->food_sources[rand_food_index]->isResourceAssignmentCorrect()) {
            this->food_sources[rand_food_index]->logEncoding();
            std::cout << "Random food index" << rand_food_index << std::endl;
        }
        assert(this->food_sources[rand_food_index]->isSequencingCorrect());
        assert(this->food_sources[rand_food_index]->isResourceAssignmentCorrect());
        this->employer_bees[i]->permute( *this->food_sources[rand_food_index], this->nrp, this->nswp, this->nmc, this->Pf, this->Pmpi, this->rengine );
    }
    for(int i = 0; i < this->nEP; ++i) {
        this->dec.decode(*(this->employer_bees[i]), true);
        if(this->employer_bees[i]->getMakespan() < this->food_sources[i]->getMakespan()) {
            this->substituteFoodSource(i, std::move(this->employer_bees[i]));
        }
    }
    //NOW I NEED ONLOOKERS PHASE
    this->onLook();
    //AND FINALLY I SHALL PRUNE OLD FOOD SOURCES
    this->renovateFoodSources();
    this->makespan_at_gen.push_back( this->getBestMakespan() );
}

void rouflex::ABC::renovateFoodSources() {
    for(int i = 0; i < this->no_improvement_cycles.size(); ++i) {
        ++this->no_improvement_cycles[i];
        if(this->no_improvement_cycles[i] >= this->limit) this->resetFoodSource(i);
    }
}

void rouflex::ABC::substituteFoodSource(const int fs, std::unique_ptr<Encoding> new_foodsource) {
    if(!new_foodsource->isResourceAssignmentCorrect()) new_foodsource->logEncoding();
    assert(new_foodsource->isSequencingCorrect());
    assert(new_foodsource->isResourceAssignmentCorrect());
    assert(fs < this->food_sources.size());
    this->food_sources[fs] = std::move(new_foodsource);
    this->no_improvement_cycles[fs] = -1;
    this->updateMakespan( this->food_sources[fs]->getMakespan() );
    if(!this->food_sources[fs]->isResourceAssignmentCorrect()) this->food_sources[fs]->logEncoding();
    assert(this->food_sources[fs]->isSequencingCorrect());
    assert(this->food_sources[fs]->isResourceAssignmentCorrect());
}

void rouflex::ABC::resetFoodSource(const int fs) {
    assert(fs < this->food_sources.size());
    this->food_sources[fs] = std::make_unique<Encoding>(this->instance, this->rengine, true, this->Pfinit);
    this->dec.decode(*(this->food_sources[fs]), true);
    this->no_improvement_cycles[fs] = 0;
    this->updateMakespan( this->food_sources[fs]->getMakespan() );
}

/* this method shall handle onlooker bees */
void rouflex::ABC::onLook() {
    int fs1, fs2, support, rand_food_index;
    std::bernoulli_distribution bd(this->Pbt);
    onlooker_origin.clear();
    for(int i = 0; i < this->nOP; ++i) {
        fs1 = this->rengine() % this->nEP;
        fs2 = this->rengine() % this->nEP;
        while( fs1 == fs2 ) fs2 = this->rengine() % this->nEP;
        if( this->food_sources[fs1]->getMakespan() > this->food_sources[fs2]->getMakespan() ) {
            //in this code portion I'd just substititute food sources indexes so as to ensure the first one is the best
            //so I exchange variables
            support = fs1;
            fs1 = fs2;
            fs2 = support;
        }
        assert( this->food_sources[fs1]->getMakespan() <= this->food_sources[fs2]->getMakespan() );
        if( bd( this->rengine ) ) this->onlooker_origin.emplace_back( fs1 );
        else this->onlooker_origin.emplace_back( fs2 );
    }
    this->onlooker_bees.clear();
    for(int i = 0; i < this->nOP; ++i) {
        this->onlooker_bees.emplace_back( std::make_unique<Encoding>(*this->food_sources[ this->onlooker_origin[i] ]) );
        rand_food_index = this->rengine() % this->nEP;
        this->onlooker_bees.back()->permute( *this->food_sources[rand_food_index], this->nrp, this->nswp, this->nmc, this->Pf, this->Pmpi, this->rengine );
        this->dec.decode(*(this->onlooker_bees[i]), true);
        if(this->onlooker_bees[i]->getMakespan() < this->food_sources[ this->onlooker_origin[i] ]->getMakespan()) {
            this->substituteFoodSource( this->onlooker_origin[i] , std::move(this->onlooker_bees[i]) );
        }
    }
}

void rouflex::ABC::makespanToCsv(const std::string & filepath) const {
    std::string result;
    std::ofstream file(filepath);
    
    result += "nIter,";
    result += "nEP,";
    result += "nOP,";
    result += "limit,";
    result += "Pbt,";
    result += "Pmpi,";
    result += "nrp,";
    result += "nswp,";
    result += "nmc,";
    result += "Pfinit,";
    result += "Pf,";
    result += "Makespan";

    result += "\n";

    result += std::to_string(this->nIter) + ",";
    result += std::to_string(this->nEP) + ",";
    result += std::to_string(this->nOP) + ",";
    result += std::to_string(this->limit) + ",";
    result += std::to_string(this->Pbt) + ",";
    result += std::to_string(this->Pmpi) + ",";
    result += std::to_string(this->nrp) + ",";
    result += std::to_string(this->nswp) + ",";
    result += std::to_string(this->nmc) + ",";
    result += std::to_string(this->Pfinit) + ",";
    result += std::to_string(this->Pf) + ",";
    result += std::to_string(this->best_makespan);

    file << result;
    file.close();
}

//ABCSetting method

timeperiod::timeunit rouflex::ABCSetting::solve(const std::string & filepath) const {
    std::default_random_engine reng;
    timeperiod::timeunit makespan;
    reng.seed(this->seed);
    ABC abc(this->instance,
            this->nIter,
            this->nEP,
            this->nOP,
            this->limit,
            this->Pbt,
            this->Pmpi,
            this->nrp,
            this->nswp,
            this->nmc,
            this->Pfinit,
            this->Pf,
            reng);
    makespan = abc.solve();
    if( ! filepath.empty() ) abc.makespanToCsv(filepath);
    return makespan;
}

//In the end we will have a function to launch parallel execution of multiple algorithms
void rouflex::abc_parallel_exec(std::vector< ABCSetting > & ABCs, std::atomic<int> & global_read_point, const std::string & folder_path) {
    int read_point = global_read_point++;
    std::string result_filepath;
    while(read_point < ABCs.size()) {
        result_filepath = folder_path + "/result" + std::to_string(read_point) + ".csv";
        std::cout << ABCs[ read_point ].solve( result_filepath ) << std::endl;
        read_point = global_read_point++;
    }
}
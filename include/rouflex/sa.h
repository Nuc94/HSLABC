#ifndef SA_H
#define SA_H

#include <cmath>
#include <memory>
#include <vector>

#include "instance.h"
#include "encoding.h"
#include "decoder.h"

namespace rouflex {

    const int sa_int = 1000;

    class SA {
    public:
        SA(
            const Instance & instance_inp,
            const double starting_temperature_inp,
            const double min_temperature_inp,
            const double alpha_inp,
            const int max_iterations_inp,
            std::default_random_engine & rengine_inp
        );
        void execute();
        inline timeperiod::timeunit getBestMakespan() const { return this->best_makespan; }
        inline int getNIterations() const { return this->n_iterations; }
        inline int getNSubstitutions() const { return this->n_substitutions; }
        inline int getNSimulations() const { return this->n_simulations; }
        void makespanToCsv(const std::string & filepath) const;
    private:
        const Instance & instance;
        const double starting_temperature;
        const double min_temperature;
        const double alpha;
        const int max_iterations;
        std::default_random_engine & rengine;
        Decoder dec;
        std::unique_ptr<Encoding> root_solution;
        timeperiod::timeunit current_makespan;
        timeperiod::timeunit best_makespan;
        double temperature;
        int n_iterations;
        int n_substitutions;
        int n_simulations;
        void iterate();
        bool exitCondition() const;
        void simulateNeighbor( std::unique_ptr<Encoding> & neighbor );
        bool shallSubstituteSolution( const timeperiod::timeunit neighbor_makespan );
        void substituteSolution( std::unique_ptr<Encoding> & neighbor );
        void handleNeighbor( std::unique_ptr<Encoding> & neighbor );
        void updateTemperature();
        void initializeRootSolution();
    };

}

#endif //SA_H
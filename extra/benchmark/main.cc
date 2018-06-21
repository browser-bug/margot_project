#include <iostream>
#include <cstdlib>
#include <chrono>

#include "evaluator.hpp"

#include "test_add_ops.hpp"
#include "test_select_data_feature.hpp"
#include "test_add_constraint.hpp"
#include "test_set_rank.hpp"
#include "test_update.hpp"
#include "test_update_scaling.hpp"


int main(int argc, char* argv[])
{
  std::cout << "Running framework benchmark:" << std::endl;

  // test adding Operating Points
  {
    std::cout << "\tAdd Operating Points" << std::endl;
    Launcher<std::chrono::microseconds> l("add_ops");
    l.run< AddOperatingPoints<0> >("No Constraints");
    l.run< AddOperatingPoints<1> >("1 Constraint");
    l.run< AddOperatingPoints<2> >("2 Constraints");
    l.run< AddOperatingPoints<3> >("3 Constraints");
  }

  // test selecting a feature cluster
  {
    std::cout << "\tSelect feature cluster" << std::endl;
    Launcher<std::chrono::microseconds> l("select_features");
    l.run< SelectDataFeature<margot::FeatureDistanceType::EUCLIDEAN> >("Euclidean");
    l.run< SelectDataFeature<margot::FeatureDistanceType::NORMALIZED> >("Normalized");
  }

  // test adding a constraint
  {
    std::cout << "\tAdd a constraint" << std::endl;
    Launcher<std::chrono::microseconds> l("add_constraint");
    l.run< AddConstraint<true> >("Worst case");
    l.run< AddConstraint<false> >("Best case");
  }

  // test setting the rank
  {
    std::cout << "\tSet the objective function" << std::endl;
    Launcher<std::chrono::microseconds> l("set_rank");
    l.run< SetRank<true> >("Worst case");
    l.run< SetRank<false> >("Best case");
  }

  // test solving the optimization problem
  {
    std::cout << "\tSolve the optimization problem flat" << std::endl;
    Launcher<std::chrono::microseconds> l("update_flat");
    l.run< UpdateFlat<0> >("No changes");
    l.run< UpdateFlat<5> >("5 Operating Points");
    l.run< UpdateFlat<50> >("50 Operating Points");
    l.run< UpdateFlat<100> >("100 Operating Points");
  }


  // test solving the optimization problem worst case
  {
    std::cout << "\tSolve the optimization problem worst case" << std::endl;
    Launcher<std::chrono::microseconds> l("update_worst_case");
    l.run< UpdateScaling<1> >("1 constraint");
    l.run< UpdateScaling<2> >("2 constraints");
    l.run< UpdateScaling<3> >("3 constraints");
    l.run< UpdateScaling<4> >("4 constraints");
  }

  return EXIT_SUCCESS;
}

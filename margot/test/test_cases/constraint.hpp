#include <vector>

#include <cxxtest/TestSuite.h>

#include <margot/constraint.hpp>
#include <margot/goal.hpp>
#include <margot/knowledge_adaptor.hpp>
#include <margot/knowledge_base.hpp>
#include <margot/monitor.hpp>

class Constraint : public CxxTest::TestSuite {
  using software_knob_geometry = margot::OperatingPointSegment<2, margot::Data<int> >;
  using metrics_geometry = margot::OperatingPointSegment<2, margot::Distribution<float> >;
  using MyOperatingPoint = margot::OperatingPoint<software_knob_geometry, metrics_geometry>;
  using OPStream = typename margot::Knowledge<MyOperatingPoint>::OPStream;

  std::vector<MyOperatingPoint> op_list;

  margot::Knowledge<MyOperatingPoint> kb;
  margot::KnowledgeAdaptor<MyOperatingPoint, float> adaptor;
  margot::Goal<float, margot::ComparisonFunctions::GREATER> goal;
  margot::Monitor<float> monitor;

 public:
  void setUp(void) {
    op_list.push_back({{1, 2}, {margot::Distribution<float>(3, 0.1), margot::Distribution<float>(4, 0.1)}});
    op_list.push_back({{2, 3}, {margot::Distribution<float>(5, 0.1), margot::Distribution<float>(6, 0.1)}});
    op_list.push_back({{3, 4}, {margot::Distribution<float>(7, 0.1), margot::Distribution<float>(8, 0.1)}});

    // populate the knowledge base
    for (const auto& op : op_list) {
      kb.add(op);
    }

    // set the monitor for the knowledge base
    adaptor.emplace<margot::OperatingPointSegments::METRICS, 0, 1>(monitor);
  }

  void tearDown(void) {
    op_list.clear();
    kb.clear();
    adaptor.clear();
  }

  void test_creation(void) {
    // set the goal to lower level
    goal = margot::Goal<float, margot::ComparisonFunctions::GREATER>(2);

    // set up of the constraint
    margot::Constraint<MyOperatingPoint, margot::OperatingPointSegments::METRICS, 0, 0, decltype(goal)>
        constraint(goal);
    constraint.set(kb);
    constraint.set_field_adaptor(adaptor);

    // check that we have no blocking knowledge
    TS_ASSERT(constraint.get_closest().empty())

    // add all the Operating Point from the list
    OPStream output_ops;
    OPStream input_ops = kb.to_stream();
    constraint.filter_add(input_ops, output_ops);

    // check that we have no blocking knowledge
    TS_ASSERT(constraint.get_closest().empty())
  }
};

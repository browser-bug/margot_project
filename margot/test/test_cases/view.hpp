#include <vector>

#include <cxxtest/TestSuite.h>

#include <margot/knowledge_base.hpp>
#include <margot/view.hpp>

class View : public CxxTest::TestSuite {
  using software_knob_geometry = margot::OperatingPointSegment<2, margot::Data<int> >;
  using metrics_geometry = margot::OperatingPointSegment<2, margot::Distribution<float> >;
  using MyOperatingPoint = margot::OperatingPoint<software_knob_geometry, metrics_geometry>;

  std::vector<MyOperatingPoint> op_list;

 public:
  void setUp(void) {
    op_list.push_back({{1, 2}, {margot::Distribution<float>(3, 0.1), margot::Distribution<float>(4, 0.1)}});
    op_list.push_back({{2, 3}, {margot::Distribution<float>(5, 0.1), margot::Distribution<float>(6, 0.1)}});
    op_list.push_back({{3, 4}, {margot::Distribution<float>(7, 0.1), margot::Distribution<float>(8, 0.1)}});
  }

  void tearDown(void) { op_list.clear(); }

  void test_creation(void) {
    margot::Knowledge<MyOperatingPoint> kb;

    for (const auto& op : op_list) {
      kb.add(op);
    }

    using first_metric_1sigma =
        margot::OPField<margot::OperatingPointSegments::METRICS, margot::BoundType::LOWER, 0, 1>;
    margot::View<MyOperatingPoint, margot::FieldComposer::SIMPLE, first_metric_1sigma> view(1.0f);

    TS_ASSERT(view.empty());
    TS_ASSERT_EQUALS(view.size(), 0);
    TS_ASSERT(!view.front());
    TS_ASSERT(!view.back());

    using OPStream = typename decltype(view)::OPStream;

    OPStream ops = view.range();
    TS_ASSERT(ops.empty());
    ops = view.range(1, 5);
    TS_ASSERT(ops.empty());
  }

  void test_constraint_like(void) {
    margot::Knowledge<MyOperatingPoint> kb;

    for (const auto& op : op_list) {
      kb.add(op);
    }

    using first_metric_1sigma =
        margot::OPField<margot::OperatingPointSegments::METRICS, margot::BoundType::LOWER, 0, 1>;
    margot::View<MyOperatingPoint, margot::FieldComposer::SIMPLE, first_metric_1sigma> view(1.0f);

    view.add(kb.begin(), kb.end());

    TS_ASSERT(!view.empty());
    TS_ASSERT_EQUALS(view.size(), 3);
    TS_ASSERT_EQUALS(*view.front(), op_list[0]);
    TS_ASSERT_EQUALS(*view.back(), op_list[2]);

    using OPStream = typename decltype(view)::OPStream;

    OPStream ops = view.range();
    TS_ASSERT(!ops.empty());
    TS_ASSERT_EQUALS(ops.size(), 3)
    ops = view.range(1, 5);
    TS_ASSERT(!ops.empty());
    TS_ASSERT_EQUALS(ops.size(), 2);
    TS_ASSERT_EQUALS(*ops[0], op_list[0]);
    TS_ASSERT_EQUALS(*ops[1], op_list[1]);
  }

  void test_rank_like(void) {
    margot::Knowledge<MyOperatingPoint> kb;

    for (const auto& op : op_list) {
      kb.add(op);
    }

    using first_metric_1sigma =
        margot::OPField<margot::OperatingPointSegments::METRICS, margot::BoundType::UPPER, 0, 1>;
    using second_metric_1sigma =
        margot::OPField<margot::OperatingPointSegments::METRICS, margot::BoundType::UPPER, 1, 1>;
    margot::View<MyOperatingPoint, margot::FieldComposer::LINEAR, first_metric_1sigma, second_metric_1sigma>
        view(1.0f, 1.0f);

    view.add(kb.begin(), kb.end());

    TS_ASSERT(!view.empty());
    TS_ASSERT_EQUALS(view.size(), 3);
    TS_ASSERT_EQUALS(*view.front(), op_list[0]);
    TS_ASSERT_EQUALS(*view.back(), op_list[2]);

    using OPStream = typename decltype(view)::OPStream;

    OPStream ops = view.range();
    TS_ASSERT(!ops.empty());
    TS_ASSERT_EQUALS(ops.size(), 3)
    ops = view.range(7, 12);
    TS_ASSERT(!ops.empty());
    TS_ASSERT_EQUALS(ops.size(), 2);
    TS_ASSERT_EQUALS(*ops[0], op_list[0]);
    TS_ASSERT_EQUALS(*ops[1], op_list[1]);
  }
};

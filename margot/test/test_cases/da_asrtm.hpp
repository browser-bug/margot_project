#include <vector>

#include <cxxtest/TestSuite.h>

#include <margot/asrtm.hpp>
#include <margot/da_asrtm.hpp>
#include <margot/goal.hpp>

class DataAwareAsrtm : public CxxTest::TestSuite {
  using software_knob_geometry = margot::OperatingPointSegment<1, margot::Data<int> >;
  using metrics_geometry = margot::OperatingPointSegment<3, margot::Distribution<float> >;
  using MyOperatingPoint = margot::OperatingPoint<software_knob_geometry, metrics_geometry>;
  using MyAsrtm = margot::Asrtm<MyOperatingPoint>;

  std::vector<MyOperatingPoint> op_list_2;
  std::vector<MyOperatingPoint> op_list_5;
  std::vector<MyOperatingPoint> op_list_7;

  margot::Goal<float, margot::ComparisonFunctions::GREATER> greater_goal;
  margot::Goal<float, margot::ComparisonFunctions::GREATER_OR_EQUAL> greater_or_equal;
  margot::Goal<float, margot::ComparisonFunctions::LESS> less_goal;

  // definition for rank
  using avg_knob_0 =
      margot::OPField<margot::OperatingPointSegments::SOFTWARE_KNOBS, margot::BoundType::LOWER, 0, 0>;
  using avg_metric_0 =
      margot::OPField<margot::OperatingPointSegments::METRICS, margot::BoundType::LOWER, 0, 0>;
  using avg_metric_1 =
      margot::OPField<margot::OperatingPointSegments::METRICS, margot::BoundType::LOWER, 1, 0>;
  using avg_metric_2 =
      margot::OPField<margot::OperatingPointSegments::METRICS, margot::BoundType::LOWER, 2, 0>;

 public:
  void setUp(void) {
    op_list_2 = std::vector<MyOperatingPoint>(
        {{{1},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(1, 0.1),
           margot::Distribution<float>(7, 0.1)}},
         {{2},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(2, 0.1),
           margot::Distribution<float>(6, 0.1)}}});

    op_list_5 = std::vector<MyOperatingPoint>(
        {{{3},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(3, 0.1),
           margot::Distribution<float>(5, 0.1)}},
         {{4},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(4, 0.1),
           margot::Distribution<float>(4, 0.1)}},
         {{5},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(5, 0.1),
           margot::Distribution<float>(3, 0.1)}},
         {{6},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(6, 0.1),
           margot::Distribution<float>(2, 0.1)}},
         {{7},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(7, 0.1),
           margot::Distribution<float>(1, 0.1)}}});

    op_list_7 = std::vector<MyOperatingPoint>(
        {{{1},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(1, 0.1),
           margot::Distribution<float>(7, 0.1)}},
         {{2},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(2, 0.1),
           margot::Distribution<float>(6, 0.1)}},
         {{3},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(3, 0.1),
           margot::Distribution<float>(5, 0.1)}},
         {{4},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(4, 0.1),
           margot::Distribution<float>(4, 0.1)}},
         {{5},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(5, 0.1),
           margot::Distribution<float>(3, 0.1)}},
         {{6},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(6, 0.1),
           margot::Distribution<float>(2, 0.1)}},
         {{7},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(7, 0.1),
           margot::Distribution<float>(1, 0.1)}}});
  }

  void tearDown(void) {
    op_list_2.clear();
    op_list_5.clear();
    op_list_7.clear();
  }

  void test_creation_empty(void) {
    margot::DataAwareAsrtm<MyAsrtm, int, margot::FeatureDistanceType::EUCLIDEAN,
                           margot::FeatureComparison::DONT_CARE>
        manager;
    manager.add_feature_cluster({{4}});
    manager.add_feature_cluster({{2}});
  }

  void test_creation_before(void) {
    // prepare everything
    margot::DataAwareAsrtm<MyAsrtm, int, margot::FeatureDistanceType::EUCLIDEAN,
                           margot::FeatureComparison::DONT_CARE>
        manager;
    manager.add_feature_cluster({{5}});
    manager.add_feature_cluster({{2}});
    manager.create_new_state("default");
    manager.change_active_state("default");
    manager.set_rank<margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_metric_2>(1.0f);
    manager.select_feature_cluster({{2}});
    manager.add_operating_points(op_list_2);
    manager.select_feature_cluster({{5}});
    manager.add_operating_points(op_list_5);

    // check the result for feature cluster 2
    manager.select_feature_cluster({{2}});
    manager.find_best_configuration();
    TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 2);

    // check the result for feature cluster 5
    manager.select_feature_cluster({{5}});
    manager.find_best_configuration();
    TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 7);
  }

  void test_creation_after(void) {
    // prepare everything
    margot::DataAwareAsrtm<MyAsrtm, int, margot::FeatureDistanceType::EUCLIDEAN,
                           margot::FeatureComparison::DONT_CARE>
        manager;
    manager.add_feature_cluster({{5}});
    manager.select_feature_cluster({{5}});
    manager.create_new_state("default");
    manager.change_active_state("default");
    manager.set_rank<margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_metric_2>(1.0f);
    manager.add_operating_points(op_list_5);
    manager.add_feature_cluster({{2}});
    manager.select_feature_cluster({{2}});
    manager.add_operating_points(op_list_2);

    // check the result for feature cluster 2
    manager.select_feature_cluster({{2}});
    manager.find_best_configuration();
    TS_ASSERT_EQUALS(manager.get_number_operating_points(), 2);
    TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 2);

    // check the result for feature cluster 4
    manager.select_feature_cluster({{5}});
    manager.find_best_configuration();
    TS_ASSERT_EQUALS(manager.get_number_operating_points(), 5);
    TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 7);
  }

  void test_creation_before_full(void) {
    // prepare everything
    margot::DataAwareAsrtm<MyAsrtm, int, margot::FeatureDistanceType::EUCLIDEAN,
                           margot::FeatureComparison::DONT_CARE>
        manager;
    manager.add_feature_cluster({{5}});
    manager.add_feature_cluster({{2}});
    manager.create_new_state("default");
    manager.change_active_state("default");
    manager.set_rank<margot::RankObjective::MAXIMIZE, margot::FieldComposer::SIMPLE, avg_metric_2>(1.0f);
    greater_goal.set(3);
    manager.add_constraint<margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0>(greater_goal, 10);
    manager.select_feature_cluster({{5}});
    manager.add_operating_points(op_list_5);
    manager.select_feature_cluster({{2}});
    manager.add_operating_points(op_list_2);

    // check the result for feature cluster 2
    manager.select_feature_cluster({{2}});
    manager.find_best_configuration();
    TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 2);

    // check the result for feature cluster 5
    manager.select_feature_cluster({{5}});
    manager.find_best_configuration();
    TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 4);
  }

  void test_creation_after_full(void) {
    // prepare everything
    margot::DataAwareAsrtm<MyAsrtm, int, margot::FeatureDistanceType::EUCLIDEAN,
                           margot::FeatureComparison::DONT_CARE>
        manager;
    manager.add_feature_cluster({{5}});
    manager.create_new_state("default");
    manager.change_active_state("default");
    manager.set_rank<margot::RankObjective::MAXIMIZE, margot::FieldComposer::SIMPLE, avg_metric_2>(1.0f);
    greater_goal.set(3);
    manager.add_constraint<margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0>(greater_goal, 10);
    manager.select_feature_cluster({{5}});
    manager.add_operating_points(op_list_5);
    manager.add_feature_cluster({{2}});
    manager.select_feature_cluster({{2}});
    manager.add_operating_points(op_list_2);

    // check the result for feature cluster 2
    manager.select_feature_cluster({{2}});
    manager.find_best_configuration();
    TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 2);

    // check the result for feature cluster 5
    manager.select_feature_cluster({{5}});
    manager.find_best_configuration();
    TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 4);
  }
};

#ifndef MARGOT_SWITCH_FEATURE_CLUSTER_HDR
#define MARGOT_SWITCH_FEATURE_CLUSTER_HDR


#include "margot/da_asrtm.hpp"

#include "evaluator.hpp"



template< margot::FeatureDistanceType distance_type >
class SelectDataFeature
{
    using software_knob_geometry = margot::OperatingPointSegment< 1, margot::Data<int> >;
    using metrics_geometry = margot::OperatingPointSegment< 3, margot::Distribution<float> >;
    using MyOperatingPoint = margot::OperatingPoint< software_knob_geometry, metrics_geometry >;
    using avg_metric_0 = margot::OPField<margot::OperatingPointSegments::METRICS, margot::BoundType::LOWER, 0, 0>;

    std::vector< MyOperatingPoint > op_list;
    margot::DataAwareAsrtm<margot::Asrtm<MyOperatingPoint>, int, distance_type, margot::FeatureComparison::DONT_CARE, margot::FeatureComparison::DONT_CARE, margot::FeatureComparison::DONT_CARE> manager;
    margot::Goal< float, margot::ComparisonFunctions::GREATER > goal;

    const int number_of_clusters;

  public:

    SelectDataFeature( const int number_of_clusters ): number_of_clusters(number_of_clusters)
    {
      const int number_operating_points = 5;

      // reserve the space in the Operating Point list
      op_list.reserve(number_operating_points);

      // create the Operating Point list
      for (int i = 0; i < number_operating_points; ++i)
      {
        op_list.push_back(
        {
          {i},
          {margot::Distribution<float>(i, 0.1), margot::Distribution<float>(1, 0.1), margot::Distribution<float>(7, 0.1)}
        }
        );
      }

      // create the feature cluster
      for ( int i = 0; i < number_of_clusters; ++i )
      {
        manager.add_feature_cluster({{i, i + 1, i + 2}});
      }

      manager.select_feature_cluster({{0, 1, 2}});


      // define the state of the Asrtm
      manager.create_new_state("optimization");
      manager.change_active_state("optimization");
      goal.set(static_cast<float>(number_operating_points / 2));

    }


    inline void operator()( void )
    {
      manager.select_feature_cluster({{number_of_clusters - 3, number_of_clusters - 2, number_of_clusters - 1}});
    }

};


#endif // MARGOT_SWITCH_FEATURE_CLUSTER_HDR

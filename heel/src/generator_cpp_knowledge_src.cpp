/* mARGOt HEEL library
 * Copyright (C) 2018 Politecnico di Milano
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <algorithm>
#include <cstdint>

#include <heel/generator_cpp_knowledge_src.hpp>
#include <heel/generator_utils.hpp>
#include <heel/model_application.hpp>

namespace margot {
namespace heel {

cpp_source_content knowledge_cpp_content(application_model& app) {
  cpp_source_content c;
  c.required_headers.emplace_back("margot/application_geometry.hpp");
  c.required_headers.emplace_back("margot/application_knowledge.hpp");
  c.required_headers.emplace_back("margot/managers_definition.hpp");

  // as always, we can consider each block of code independent
  std::for_each(app.blocks.begin(), app.blocks.end(), [&c](block_model& block) {
    // for convenience, define a lambda that join the average value of the feature fields in a string. In this
    // way it is possible to identify easily if two features are different
    const auto str = [](const std::vector<operating_point_value>& f) {
      return join(f.begin(), f.end(), ",", [](const operating_point_value& m) { return m.mean; });
    };

    // since the operating point in margot does not have information about the feature cluster, we need to
    // cluster them manually at configure time
    std::sort(block.ops.begin(), block.ops.end(),
              [&str](const operating_point_model& a, const operating_point_model& b) {
                return str(a.features) < str(b.features);
              });

    // moreover, we ned to check if the metric segment of the Operating Point is a distribution or not
    const bool metric_is_distribution =
        std::any_of(block.metrics.begin(), block.metrics.end(),
                    [](const metric_model& metric) { return metric.distribution; });

    // define the signature of the function that adds the application knowledge (and feature clusters) to the
    // manager (if any)
    if (!block.knobs.empty()) {
      c.content << "void margot::add_application_knowledge(margot::" << block.name
                << "::manager_type& manager) {" << std::endl;

      // check if we have to generate a fake cluster, to be able to define the extra-functional requirements
      const bool is_with_features = !block.features.fields.empty();
      const bool is_with_knowledge = !block.ops.empty();
      if (is_with_features && !is_with_knowledge) {
        const std::string fake_cluster = join(block.features.fields.begin(), block.features.fields.end(), ",",
                                              [](const feature_model&) { return "1"; });
        c.content << "\tmanager.add_feature_cluster({{" << fake_cluster << "}});" << std::endl;
        c.content << "\tmanager.select_feature_cluster({{" << fake_cluster << "}});" << std::endl;
      } else if (is_with_knowledge) {
        // if we reach this statement it means that we have to generate the code that adds clusters (if any)
        // and Operating Point to the block manager.
        std::string current_cluster = "";
        bool first_iteration = true;
        std::for_each(block.ops.begin(), block.ops.end(), [&](const operating_point_model& op) {
          // check what we need to generate (beside the operating point)
          const bool is_feature_cluster_different = str(op.features).compare(current_cluster) != 0;
          const bool need_to_generate_change_cluster = is_with_features && is_feature_cluster_different;
          const bool need_to_generate_add_op_begin = first_iteration || is_feature_cluster_different;
          const bool need_to_generate_add_op_end =
              is_feature_cluster_different && is_with_features && !first_iteration;
          first_iteration = false;

          // check if we need to update the current cluster of features
          if (is_feature_cluster_different) {
            current_cluster = str(op.features);
          }

          // emit the cpp code according to the decision logic
          if (need_to_generate_add_op_end) {
            c.content << "\t});" << std::endl;
          }
          if (need_to_generate_change_cluster) {
            c.content << "\tmanager.add_feature_cluster({{" << current_cluster << "}});" << std::endl;
            c.content << "\tmanager.select_feature_cluster({{" << current_cluster << "}});" << std::endl;
          }
          if (need_to_generate_add_op_begin) {
            c.content << "\tmanager.add_operating_points(std::vector<margot::" << block.name
                      << "::operating_point_type>({" << std::endl;
          }

          // now we can generate the operating point code
          c.content << "\t\t{ // new operating point" << std::endl;
          c.content << "\t\t\t{ // software knobs " << std::endl << "\t\t\t\t";
          std::size_t counter = 0;
          c.content << join(block.knobs.begin(), block.knobs.end(), ", ", [&](const knob_model& knob) {
            const auto t = counter++;
            return knob.type.compare("string") != 0 ? op.knobs[t].mean
                                                    : "margot::" + block.name + "::knob_" + knob.name +
                                                          "_to_val(\"" + op.knobs[t].mean + "\")";
          }) << std::endl;
          c.content << "\t\t\t}," << std::endl;
          c.content << "\t\t\t{ // extra-functional properties " << std::endl << "\t\t\t\t";
          counter = 0;
          c.content << join(block.metrics.begin(), block.metrics.end(), ", ",
                            [&](const metric_model& metric) {
                              const auto t = counter++;
                              const std::string stdv = !metric.distribution && metric_is_distribution
                                                           ? std::string("0")
                                                           : op.metrics[t].standard_deviation;
                              return metric_is_distribution ? "margot::" + block.name + "::metrics_type(" +
                                                                  op.metrics[t].mean + "," + stdv + ")"
                                                            : op.metrics[t].mean;
                            })
                    << std::endl;
          c.content << "\t\t\t}" << std::endl;
          c.content << "\t\t}," << std::endl;
        });

        // now we can close the add operating point function
        c.content << "\t}));" << std::endl << std::endl << std::endl;
      }

      c.content << "}" << std::endl;
    }
  });
  return c;
}

}  // namespace heel
}  // namespace margot
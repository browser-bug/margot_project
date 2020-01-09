#include <algorithm>
#include <cstdint>
#include <string>

#include <heel/cpp_enum_conversion.hpp>
#include <heel/cpp_state_emitter.hpp>
#include <heel/cpp_utils.hpp>
#include <heel/generator_utils.hpp>
#include <heel/model_state.hpp>

namespace margot {
namespace heel {

cpp_source_content generate_cpp_content(const state_model& state, const std::string block_name) {
  // define a lambda that generates the margot field type, which is the basic rank component
  const auto gen_field = [&block_name](const rank_field_model& field) {
    return "margot::OPField<" + cpp_enum::get(field.kind) + ",BoundType::LOWER," +
           generate_field_getter(field.kind, field.name, block_name) + ",0>";
  };

  // now it's time to actually define state
  cpp_source_content c;
  c.required_headers.emplace_back("margot/application_geometry.hpp");
  c.content << "// initialization code for state <--{" << state.name << "}-->" << std::endl;
  c.content << "manager.create_new_state(\"" << state.name << "\");" << std::endl;
  c.content << "manager.change_active_state(\"" << state.name << "\");" << std::endl;
  if (state.direction != rank_direction::NONE) {
    c.content << "manager.set_rank<" << cpp_enum::get(state.direction) << ','
              << cpp_enum::get(state.combination) << ','
              << join(state.rank_fields.begin(), state.rank_fields.end(), ",",
                      [&gen_field](const rank_field_model& r) { return gen_field(r); })
              << ">";  // this defines the "type" of the rank
    c.content << "("
              << join(state.rank_fields.begin(), state.rank_fields.end(), ",",
                      [](const rank_field_model& r) { return r.coefficient; })
              << ");" << std::endl;
  }
  std::size_t constraint_counter = 0;
  std::for_each(state.constraints.begin(), state.constraints.end(),
                [&c, &state, &block_name, &constraint_counter](const constraint_model& constraint) {
                  c.content << "manager.add_constraint<" << cpp_enum::get(constraint.kind) << ','
                            << generate_field_getter(constraint.kind, constraint.name, block_name) << ','
                            << constraint.confidence << '>';
                  c.content << "(goals." << generate_goal_identifier(state.name, constraint_counter) << ','
                            << constraint_counter << ");" << std::endl;
                });

  return c;
}

}  // namespace heel
}  // namespace margot
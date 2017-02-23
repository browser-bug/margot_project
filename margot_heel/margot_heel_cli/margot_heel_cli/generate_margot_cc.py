import os                                # for creating the path
import errno                             # for checking before creating a path


from .generate_utility import generate_start_monitor_signature
from .generate_utility import generate_stop_monitor_signature
from .generate_utility import generate_update_signature



dfun_translator = {
	"AVERAGE"  : "margot::DataFunction::Average",
	"VARIANCE" : "margot::DataFunction::Variance",
	"MAX"      : "margot::DataFunction::Max",
	"MIN"      : "margot::DataFunction::Min",
}


cfun_translator = {
	"GT"  : "margot::ComparisonFunction::Greater",
	"GE"  : "margot::ComparisonFunction::GreaterOrEqual",
	"LT"  : "margot::ComparisonFunction::Less",
	"LE"  : "margot::ComparisonFunction::LessOrEqual",
}

what_translator = {
	"AVERAGE"  : "average",
	"VARIANCE" : "variance",
	"MAX"      : "max",
	"MIN"      : "min",
}



def generate_block_body( block_model, op_lists, cc ):
	"""
	Generates the per block code
	"""

	# write the begin of the namespace
	cc.write('\n\n\tnamespace {0} {{\n'.format(block_model.block_name).encode('utf-8'))

	# write all the monitors
	if block_model.monitor_models:

		# open the monitor namespace
		cc.write('\n\t\tnamespace monitor {\n'.encode('utf-8'))

		# loop over the monitor
		for monitor_model in block_model.monitor_models:
			cc.write('\t\t\t{0} {1};\n'.format(monitor_model.monitor_class, monitor_model.monitor_name).encode('utf-8'))

		# close the monitor namespace
		cc.write('\t\t} // namespace monitor\n'.encode('utf-8'))

	# write all the goals
	if block_model.goal_models:

		# open the goal namespace
		cc.write('\n\t\tnamespace goal {\n'.encode('utf-8'))

		# loop over the goals
		for goal_model in block_model.goal_models:
			cc.write('\t\t\tgoal_t {0};\n'.format(goal_model.name).encode('utf-8'))

		# close the goal namespace
		cc.write('\t\t} // namespace goal\n'.encode('utf-8'))

	# write the manager
	cc.write('\n\n\t\tasrtm_t manager;\n'.encode('utf-8'))

	# write the logger
	cc.write('\n\t\t#ifdef MARGOT_LOG_FILE\n'.encode('utf-8'))
	cc.write('\t\tLogger file_logger;\n'.encode('utf-8'))
	cc.write('\t\t#endif // MARGOT_LOG_FILE\n\n\n'.encode('utf-8'))

	# write all the exposed variables from the monitors
	for monitor_model in block_model.monitor_models:
		for exposed_var_what in monitor_model.exposed_metrics:
			cc.write('\n\t\tdouble {0};\n'.format(monitor_model.exposed_metrics[exposed_var_what]).encode('utf-8'))

	# write the update function
	cc.write('\n\n\t\tbool {0}\n'.format(generate_update_signature(block_model)).encode('utf-8'))
	cc.write('\t\t{\n'.encode('utf-8'))
	cc.write('\t\t\t#ifdef BLOCK_{0}_AUTOTUNED\n'.format(block_model.block_name.upper()).encode('utf-8'))
	cc.write('\t\t\tmanager.update();\n'.encode('utf-8'))
	cc.write('\t\t\tmanager.find_best_operating_point();\n'.encode('utf-8'))
	cc.write('\t\t\tbool conf_changed = false;\n'.encode('utf-8'))
	cc.write('\t\t\tconfiguration_t actual_configuration = manager.get_best_configuration(&conf_changed);\n'.encode('utf-8'))
	cc.write('\t\t\tif (conf_changed)\n'.encode('utf-8'))
	cc.write('\t\t\t{\n'.encode('utf-8'))
	param_list = ['{0}& {1}'.format(x.var_type, x.var_name) for x in block_model.software_knobs]
	for knob in block_model.software_knobs:
		cc.write('\t\t\t\t{0} = actual_configuration[static_cast<int>(Knob::{1})];\n'.format(knob.var_name, knob.name.upper()).encode('utf-8'))
	for model_learn in block_model.learn_models:
		for model_knob in model_learn.knobs:
			cc.write('\t\t\t\t{0} = actual_configuration[static_cast<int>(Knob::{1})];\n'.format(model_knob.var_name, model_knob.name.upper()).encode('utf-8'))
	cc.write('\t\t\t}\n'.encode('utf-8'))
	cc.write('\t\t\treturn conf_changed;\n'.encode('utf-8'))
	cc.write('\t\t\t#else // BLOCK_{0}_AUTOTUNED\n'.format(block_model.block_name.upper()).encode('utf-8'))
	cc.write('\t\t\treturn false;\n'.encode('utf-8'))
	cc.write('\t\t\t#endif // BLOCK_{0}_AUTOTUNED\n'.format(block_model.block_name.upper()).encode('utf-8'))
	cc.write('\t\t}\n'.encode('utf-8'))

	# write the start_monitor function
	cc.write('\n\n\t\tvoid {0}\n'.format(generate_start_monitor_signature(block_model)).encode('utf-8'))
	cc.write('\t\t{\n'.encode('utf-8'))
	for monitor_model in block_model.monitor_models:
		if monitor_model.start_method:
			cc.write('\t\t\tmonitor::{0}.{1}('.format(monitor_model.monitor_name, monitor_model.start_method).encode('utf-8'))
			possible_arguments = [x.var_name for x in monitor_model.start_parameters if x.var_name]
			possible_arguments.extend([str(x.param_value) for x in monitor_model.start_parameters if x.param_value])
			cc.write(','.join(possible_arguments).encode('utf-8'))
			cc.write(');\n'.encode('utf-8'))
	cc.write('\t\t}\n'.encode('utf-8'))


	# write the stop_monitor function
	cc.write('\n\n\t\tvoid {0}\n'.format(generate_stop_monitor_signature(block_model)).encode('utf-8'))
	cc.write('\t\t{\n'.encode('utf-8'))
	for monitor_model in block_model.monitor_models:
		if monitor_model.stop_method:
			cc.write('\t\t\tmonitor::{0}.{1}('.format(monitor_model.monitor_name, monitor_model.stop_method).encode('utf-8'))
			possible_arguments = [x.var_name for x in monitor_model.stop_parameters if x.var_name]
			possible_arguments.extend([str(x.param_value) for x in monitor_model.stop_parameters if x.param_value])
			cc.write(','.join(possible_arguments).encode('utf-8'))
			cc.write(');\n'.encode('utf-8'))
	cc.write('\n'.encode('utf-8'))
	cc.write('\t\t}'.encode('utf-8'))


	# write the log function
	cc.write('\n\n\t\tvoid log( void )\n'.encode('utf-8'))
	cc.write('\t\t{\n'.encode('utf-8'))
	for monitor_model in block_model.monitor_models:
		for exposed_metric_name in monitor_model.exposed_metrics:
			cc.write('\t\t\t{0} = monitor::{1}.{2}();\n'.format(monitor_model.exposed_metrics[exposed_metric_name], monitor_model.monitor_name, what_translator[exposed_metric_name.upper()]).encode('utf-8'))
	cc.write('\n'.encode('utf-8'))
	cc.write('\n\t\t\t#ifdef MARGOT_LOG_FILE\n'.encode('utf-8'))
	cc.write('\t\t\tfile_logger.write(  \n'.encode('utf-8'))
	if block_model.block_name in op_lists.keys():
		for knob in block_model.software_knobs:
			if knob.name in op_lists[block_model.block_name].translator:
				cc.write('\t\t\t\tmargot::{0}::knob_{2}_int2str[static_cast<int>(margot::{0}::manager.get_parameter_value(static_cast<margot::field_name_t>(margot::{0}::Knob::{1})))],\n'.format(block_model.block_name, knob.name.upper(), knob.name.lower()).encode('utf-8'))
			else:
				cc.write('\t\t\t\tmargot::{0}::manager.get_parameter_value(static_cast<margot::field_name_t>(margot::{0}::Knob::{1})),\n'.format(block_model.block_name, knob.name.upper()).encode('utf-8'))
		for model_learn in block_model.learn_models:
			for model_knob in model_learn.knobs:
				cc.write('\t\t\t\tmargot::{0}::manager.get_parameter_value(static_cast<margot::field_name_t>(margot::{0}::Knob::{1})),\n'.format(block_model.block_name, model_knob.name.upper()).encode('utf-8'))
		for metric_name in op_lists[block_model.block_name].ops[0].metrics:
			cc.write('\t\t\t\tmargot::{0}::manager.get_metric_value(static_cast<margot::field_name_t>(margot::{0}::Metric::{1})),\n'.format(block_model.block_name, metric_name.upper()).encode('utf-8'))
	for goal in block_model.goal_models:
		cc.write('\t\t\t\tmargot::{0}::goal::{1}.get(),\n'.format(block_model.block_name, goal.name).encode('utf-8'))
	for monitor_model in block_model.monitor_models:
		for exposed_var_what in monitor_model.exposed_metrics:
			cc.write('\t\t\t\tmargot::{0}::{1},\n'.format(block_model.block_name, monitor_model.exposed_metrics[exposed_var_what]).encode('utf-8'))
	cc.seek(-2, 1)
	cc.write('\n\t\t\t);\n'.encode('utf-8'))
	cc.write('\t\t\t#endif // MARGOT_LOG_FILE\n'.encode('utf-8'))
	cc.write('\n\t\t\t#ifdef MARGOT_LOG_STDOUT\n'.encode('utf-8'))
	cc.write('\t\t\tstd::cout <<'.encode('utf-8'))
	cc.write('"Monitored values: "\n'.encode('utf-8'))
	for monitor_model in block_model.monitor_models:
		for exposed_var_what in monitor_model.exposed_metrics:
			cc.write('\t\t\t\t<< "[ {1} = " << margot::{0}::{1} << "]"\n'.format(block_model.block_name, monitor_model.exposed_metrics[exposed_var_what]).encode('utf-8'))
	cc.write('\t\t\t\t<< std::endl;\n'.encode('utf-8'))
	if block_model.goal_models:
		cc.write('\t\t\tstd::cout << "Goal values: "\n'.encode('utf-8'))
	for goal in block_model.goal_models:
		cc.write('\t\t\t\t<< "[ {1} = " << margot::{0}::goal::{1}.get() << "]"\n'.format(block_model.block_name, goal.name).encode('utf-8'))
	if block_model.goal_models:
		cc.write('\t\t\t\t<< std::endl;\n'.encode('utf-8'))
	if block_model.block_name in op_lists.keys():
		cc.write('\t\t\tstd::cout << "Knob values: "\n'.encode('utf-8'))
		for knob in block_model.software_knobs:
			if knob.name in op_lists[block_model.block_name].translator:
				cc.write('\t\t\t\t<< "[ {1} = " << margot::{0}::knob_{2}_int2str[static_cast<int>(margot::{0}::manager.get_parameter_value(static_cast<margot::field_name_t>(margot::{0}::Knob::{1})))] << "]"\n'.format(block_model.block_name, knob.name.upper(), knob.name.lower()).encode('utf-8'))
			else:
				cc.write('\t\t\t\t<< "[ {1} = " << margot::{0}::manager.get_parameter_value(static_cast<margot::field_name_t>(margot::{0}::Knob::{1})) << "]"\n'.format(block_model.block_name, knob.name.upper()).encode('utf-8'))
		for model_learn in block_model.learn_models:
			for model_knob in model_learn.knobs:
				cc.write('\t\t\t\t<< "[ {1} = " << margot::{0}::manager.get_parameter_value(static_cast<margot::field_name_t>(margot::{0}::Knob::{1})) << "]"\n'.format(block_model.block_name, model_knob.name.upper()).encode('utf-8'))
		cc.write('\t\t\t\t<< std::endl;\n'.encode('utf-8'))
		cc.write('\t\t\tstd::cout << "Known metrics: "\n'.encode('utf-8'))
		for metric_name in op_lists[block_model.block_name].ops[0].metrics:
			cc.write('\t\t\t\t<< "[ {1} = " << margot::{0}::manager.get_metric_value(static_cast<margot::field_name_t>(margot::{0}::Metric::{1})) << "]"\n'.format(block_model.block_name, metric_name.upper()).encode('utf-8'))
		cc.write('\t\t\t\t<< std::endl;\n'.encode('utf-8'))
	cc.write('\t\t\t#endif // MARGOT_LOG_STDOUT\n'.encode('utf-8'))
	cc.write('\t\t}\n'.encode('utf-8'))

	# write the end of the namespace
	cc.write('\n\t}} // namespace {0}\n'.format(block_model.block_name).encode('utf-8'))









def generate_margot_cc( block_models, op_lists, output_folder ):
	"""
	This function generates the actual margot source files
	"""


	# open the output file
	with open(os.path.join(output_folder, 'margot.cc'), 'wb') as cc:

		# write the include
		cc.write('#include "margot.hpp"\n'.encode('utf-8'))
		cc.write('#include "margot_op_struct.hpp"\n'.encode('utf-8'))
		cc.write('#ifdef MARGOT_LOG_STDOUT\n'.encode('utf-8'))
		cc.write('#include <iostream>\n'.encode('utf-8'))
		cc.write('#endif // MARGOT_LOG_STDOUT\n'.encode('utf-8'))
		cc.write('#ifdef MARGOT_LOG_FILE\n'.encode('utf-8'))
		cc.write('#include "margot_logger.hpp"\n'.encode('utf-8'))
		cc.write('#endif // MARGOT_LOG_FILE\n'.encode('utf-8'))

		# write the disclaimer
		cc.write('\n\n\n/**\n'.encode('utf-8'))
		cc.write(' * WARNING:\n'.encode('utf-8'))
		cc.write(' * This file is autogenerated from the "margotcli" utility function.\n'.encode('utf-8'))
		cc.write(' * Any changes to this file might be overwritten, thus in order to \n'.encode('utf-8'))
		cc.write(' * perform a permanent change, please update the configuration file \n'.encode('utf-8'))
		cc.write(' * and re-generate this file. \n'.encode('utf-8'))
		cc.write(' */ \n\n\n'.encode('utf-8'))


		# write the margot namespace begin
		cc.write('namespace margot {\n'.encode('utf-8'))


		# generate the code block-specific code
		for block_name in block_models:
			generate_block_body(block_models[block_name], op_lists, cc)




		# ----------- generate the init function signature
		cc.write('\n\n\tvoid init( '.encode('utf-8'))

		# get all the creation parameters
		creation_parameters = []
		monitor_models = []
		for block_name in block_models:
			monitor_models.extend( block_models[block_name].monitor_models )
		for monitor in monitor_models:
			creation_parameters.extend( monitor.creation_parameters )

		# compose the parameter list
		signature = ', '.join(['{0} {1}'.format(x.var_type, x.var_name) for x in creation_parameters if x.var_name])
		if not signature:
			signature = 'void'

		cc.write('{0} )\n'.format(signature).encode('utf-8'))



		# ----------- generate the init function body
		cc.write('\t{\n'.encode('utf-8'))

		# loop over the blocks
		for block_name in block_models:

			# writing a preamble
			cc.write('\n\n\t\t// --------- Initializing the block "{0}"\n'.format(block_name.upper()).encode('utf-8'))

			# get the reference to the block
			block_model = block_models[block_name]

			# loop over the monitors of the block
			for monitor_model in block_model.monitor_models:
				cc.write('\t\t{0}::monitor::{1} = {2}('.format(block_name,monitor_model.monitor_name, monitor_model.monitor_class).encode('utf-8'))
				creation_params = [x.var_name for x in monitor_model.creation_parameters if x.var_name]
				creation_params.extend( [str(x.param_value) for x in monitor_model.creation_parameters if x.param_value] )
				cc.write('{0});\n'.format(', '.join(creation_params)).encode('utf-8'))

			# loop over the goals
			for goal_model in block_model.goal_models:
				if goal_model.goal_type == 'DYNAMIC':
					cc.write('\t\t{0}::goal::{1} = margot::goal_t({0}::monitor::{2}, {3}, {4}, static_cast<{5}>({6}));\n'.format(
					block_name,
					goal_model.name,
					goal_model.monitor_name_ref,
					dfun_translator[goal_model.dfun.upper()],
					cfun_translator[goal_model.cfun.upper()],
					goal_model.stored_type_value,
					goal_model.value
					).encode('utf-8'))
				if goal_model.metric_name_ref:
					cc.write('\t\t{0}::goal::{1} = {0}::manager.create_static_goal_metric(static_cast<margot::field_name_t>({0}::Metric::{2}), {3}, static_cast<{4}>({5}));\n'.format(
					block_name,
					goal_model.name,
					goal_model.metric_name_ref.upper(),
					cfun_translator[goal_model.cfun.upper()],
					goal_model.stored_type_value,
					goal_model.value
					).encode('utf-8'))
				if goal_model.parameter_name_ref:
					cc.write('\t\t{0}::goal::{1} = {0}::manager.create_static_goal_parameter(static_cast<margot::field_name_t>({0}::Knob::{2}), {3}, static_cast<{4}>({5}));\n'.format(
					block_name,
					goal_model.name,
					goal_model.parameter_name_ref.upper(),
					cfun_translator[goal_model.cfun.upper()],
					goal_model.stored_type_value,
					goal_model.value
					).encode('utf-8'))

			# check if we need to add Operating Points
			ops_key = op_lists.keys()
			if block_name in ops_key:
				cc.write('\t\t{0}::manager.add_operating_points({0}::op_list);\n'.format(block_name).encode('utf-8'))

			# loop over the states
			for state_model in block_model.state_models:

				# add & switch to the current state
				cc.write('\n\t\t// Defining the state "{0}"\n'.format(state_model.name).encode('utf-8'))
				cc.write('\t\t{0}::manager.add_state("{1}");\n'.format(block_name, state_model.name).encode('utf-8'))
				cc.write('\t\t{0}::manager.change_active_state("{1}");\n'.format(block_name, state_model.name).encode('utf-8'))

				# define the learning software knobs
				for model_learn in block_model.learn_models:
					if model_learn.framework_type.upper() == 'SW_UCB':
						cc.write('\t\t{0}::manager.define_learning_sw_ucb_parameters({{'.format(block_name).encode('utf-8'))
						list_of_learned_params = []
						for model_knob in model_learn.knobs:
							list_of_learned_params.append('{{ {{ {0} }}, {{ {1} }}  }}'.format(model_knob.coefficient, ', '.join(model_knob.values)))
						cc.write('{0} }}'.format(', '.join(list_of_learned_params)).encode('utf-8'))
						if not model_learn.window_size == '':
							cc.write(', {0}'.format(model_learn.window_size).encode('utf-8'))
						if not model_learn.sigma == '':
							cc.write(', {0}'.format(model_learn.sigma).encode('utf-8'))
						if not model_learn.balance_coef == '':
							cc.write(', {0}'.format(model_learn.balance_coef).encode('utf-8'))
						cc.write(');\n'.encode('utf-8'))

				# define the rank
				cc.write('\t\t{0}::manager.define_{1}_rank(margot::RankObjective::{2}, '.format(
				block_name,
				state_model.rank_type.lower(),
				state_model.rank_direction.capitalize()
				).encode('utf-8'))
				rank_components = []
				for rank_element_param_name in state_model.rank_element_param:
					rank_element_param_coef = state_model.rank_element_param[rank_element_param_name]
					rank_components.append('margot::rank_parameter_t{{static_cast<field_name_t>(margot::{0}::Knob::{1}), {2}f}}'.format(block_name, rank_element_param_name.upper(), rank_element_param_coef))
				for rank_element_metric_name in state_model.rank_element_metric:
					rank_element_metric_coef = state_model.rank_element_metric[rank_element_metric_name]
					rank_components.append('margot::rank_metric_t{{static_cast<field_name_t>(margot::{0}::Metric::{1}), {2}f}}'.format(block_name, rank_element_metric_name.upper(), rank_element_metric_coef))

				cc.write(', '.join(rank_components).encode('utf-8'))
				cc.write(' );\n'.encode('utf-8'))

				# loop over the constraints
				for constraint_model in state_model.constraint_list:
					if constraint_model.target_knob:
						cc.write('\t\t{0}::manager.add_parameter_constraint(margot::{0}::goal::{1}, static_cast<field_name_t>(margot::{0}::Knob::{2}), {3});\n'.format(
						block_name,
						constraint_model.goal_ref,
						constraint_model.target_knob.upper(),
						constraint_model.priority
						).encode('utf-8'))
					if constraint_model.target_metric:
						cc.write('\t\t{0}::manager.add_metric_constraint(margot::{0}::goal::{1}, static_cast<int>(margot::{0}::Metric::{2}), {3});\n'.format(
						block_name,
						constraint_model.goal_ref,
						constraint_model.target_metric.upper(),
						constraint_model.priority
						).encode('utf-8'))


			# switch to the active state
			if block_model.state_models:
				active_state = block_model.state_models[0]
				for state_model in block_model.state_models:
					if state_model.starting:
						active_state = state_model
						break
				cc.write('\n\t\t// Switch to the starting active state\n'.encode('utf-8'))
				cc.write('\t\t{0}::manager.change_active_state("{1}");\n'.format(block_name, active_state.name).encode('utf-8'))

			# initialize the file logger
			cc.write('\n\n\t\t#ifdef MARGOT_LOG_FILE\n'.encode('utf-8'))
			cc.write('\t\t{0}::file_logger.open("{0}.log", margot::Format::PLAIN,\n'.format(block_name).encode('utf-8'))
			if block_model.block_name in op_lists.keys():
				for knob in block_model.software_knobs:
					cc.write('\t\t\t"Knob_{0}",\n'.format(knob.name.upper()).encode('utf-8'))
				for model_learn in block_model.learn_models:
					for model_knob in model_learn.knobs:
						cc.write('\t\t\t"Knob_{0}",\n'.format(model_knob.name.upper()).encode('utf-8'))
				for metric_name in op_lists[block_model.block_name].ops[0].metrics:
					cc.write('\t\t\t"Known_Metric_{0}",\n'.format(metric_name.upper()).encode('utf-8'))
			for goal in block_model.goal_models:
				cc.write('\t\t\t"goal_{0}",\n'.format(goal.name).encode('utf-8'))
			for monitor_model in block_model.monitor_models:
				for exposed_var_what in monitor_model.exposed_metrics:
					cc.write('\t\t\t"{1}_{0}",\n'.format(exposed_var_what, monitor_model.monitor_name).encode('utf-8'))
			cc.seek(-2, 1)
			cc.write(');\n'.encode('utf-8'))
			cc.write('\t\t#endif // MARGOT_LOG_FILE\n'.encode('utf-8'))

		cc.write('\t}\n'.encode('utf-8'))

		# write some trailer spaces
		cc.write('\n\n'.encode('utf-8'))

		# write the margot namespace end
		cc.write('} // namespace margot\n\n'.encode('utf-8'))

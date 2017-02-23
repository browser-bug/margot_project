import os                                # for creating the path
import errno                             # for checking before creating a path




def generate_margot_structure_hpp( block_models, op_lists, output_folder ):
	"""
	This function generates the actual margot source files
	"""


	# open the output file
	with open(os.path.join(output_folder, 'margot_op_struct.hpp'), 'w') as cc:

		# write the include guard
		cc.write('#ifndef MARGOT_OP_STRUCTURE_H\n')
		cc.write('#define MARGOT_OP_STRUCTURE_H\n')
		cc.write('\n#include <string>\n')
		cc.write('#include <unordered_map>\n')
		cc.write('#include <margot/operating_point.hpp>')

		# write the disclaimer
		cc.write('\n\n\n/**\n')
		cc.write(' * WARNING:\n')
		cc.write(' * This file is autogenerated from the "margotcli" utility function.\n')
		cc.write(' * Any changes to this file might be overwritten, thus in order to \n')
		cc.write(' * perform a permanent change, please update the configuration file \n')
		cc.write(' * and re-generate this file. \n')
		cc.write(' */ \n\n\n')

		# write the margot namespace begin
		cc.write('namespace margot {\n')


		# loop through the op list
		for block_name in op_lists:

			# get the op list
			op_list = op_lists[block_name]

			# write the begin of the namespace
			cc.write('\n\tnamespace {0} {{\n'.format(block_name))


			# write the translation function
			for string_param_name in op_list.translator:

				# write the function to convert from str to int
				cc.write('\n\t\textern std::unordered_map<std::string,int> knob_{0}_str2int;\n'.format(string_param_name.lower()))
				cc.write('\n\t\textern std::unordered_map<int,std::string> knob_{0}_int2str;\n'.format(string_param_name.lower()))



			# get the list of sw knob and metrics
			knob_list = sorted(op_list.ops[0].knobs.keys())
			metric_list = sorted(op_list.ops[0].metrics.keys())
			learned_list = block_models[block_name].get_learned_software_knobs()
			if learned_list:
				knob_list.extend(learned_list)

			# write the enum for the op fields
			cc.write('\n\t\t// Name of the software knobs\n')
			cc.write('\t\tenum class Knob : margot::field_name_t {\n')
			for index, knob_name in enumerate(knob_list):
				cc.write('\t\t\t{0} = {1},\n'.format(knob_name.upper(), index))
			cc.write('\t\t};\n')
			cc.write('\n\t\t// Name of the metrics of interest\n')
			cc.write('\t\tenum class Metric : margot::field_name_t {\n')
			for index, metric_name in enumerate(metric_list):
				cc.write('\t\t\t{0} = {1},\n'.format(metric_name.upper(), index))
			cc.write('\t\t};\n')


			# write the end of namespace
			cc.write('\n\t}} // namespace {0}\n'.format(block_name))


		# write the margot namespace end
		cc.write('\n} // namespace margot\n\n')


		# write the header trailer
		cc.write('#endif // MARGOT_OP_STRUCTURE_H\n\n')

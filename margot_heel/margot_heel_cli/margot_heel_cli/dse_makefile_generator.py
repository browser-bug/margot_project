import os
import pickle

common_profile_name = 'profile.margot_dse'

def generate_configuration_makefile( configuration, percentage_doe, percentage_global):
	with open(os.path.join(configuration.cwd, 'Makefile'), 'w') as outfile:

		outfile.write('{0}: exec\n'.format(common_profile_name))
		outfile.write('\t@echo "[GLOBAL: {0}%, DOE: {1}%] EXEC {2}"\n'.format(percentage_global,percentage_doe, configuration.name))
		outfile.write('\t@./exec {0} > stdout.txt 2> stderr.txt || (echo \'****** PROFILE ERROR: ****** Not able to execute the program. stderr and stdout logs are in folder $(current_dir)\'&& exit 1)\n'.format(' '.join(configuration.flags)))
		outfile.write('\t@cp {0} {1}\n'.format(configuration.log_file, common_profile_name))

		outfile.write('mkfile_path:=$(abspath $(lastword $(MAKEFILE_LIST)))\n')

		outfile.write('current_dir := $(dir $(mkfile_path))\n')

	return os.path.join(configuration.cwd, common_profile_name)


#creates the doe makefile. it's intermediate in the sense that the different inputs are handled at a superior level and the doe is done only on the different parameters with a given input.
def generate_intermediate_makefile(doe, out_path, py_ops_generator, dest_flags, percentage):

	# create the dependencies
	dependencies_log_file = []
	dependencies_folder = []
	for c in doe.plan:
		log_file_path = os.path.join(c.cwd, common_profile_name)
		common_path = os.path.commonpath([log_file_path, out_path])
		dependencies_log_file.append(os.path.join('.', os.path.relpath(log_file_path, common_path)))
		dependencies_folder.append(os.path.join('.', os.path.relpath(c.cwd, common_path)))

	with open(os.path.join(out_path, 'doe.pkl'), 'wb') as outfile:
		pickle.dump(dest_flags, outfile, pickle.HIGHEST_PROTOCOL)


	# write the doe makefile
	with open(os.path.join(out_path, 'Makefile'), 'w') as outfile:

		# write the global objective of the dse
		outfile.write('dse: {0}\n'.format(' \\\n     '.join(dependencies_log_file)))
#		generator_flags = ' '.join(dest_flags)
		outfile.write('\t@echo "[GLOBAL: {0}%] of the different input processing"\n'.format(percentage))
#		outfile.write('\t@python3 {0} {1}\n'.format(py_ops_generator, generator_flags))
		outfile.write('\t@python3 {0} --data {1}\n'.format(py_ops_generator, os.path.join(out_path, 'doe.pkl')))
		outfile.write('\n\n\n')
		# write the rule to compute the dependencies
		for index_dep, dep in enumerate(dependencies_log_file):
			outfile.write('{0}:\n'.format(dep))
			outfile.write('\t@$(MAKE) -C {0}\n'.format(dependencies_folder[index_dep]))
			outfile.write('\n\n')


# TO BE TESTED YET
def generate_global_makefile(folder_list, out_path,launchpad_name, outfile_name):
	with open(os.path.join(out_path, 'Makefile'), 'w') as outfile:
		outfile.write('dse: {0}\n'.format(' \\\n     '.join(folder_list)))
		outfile.write('\t@cp {0}/0/{1} {1}\n\n'.format(launchpad_name, outfile_name))
		for folder in folder_list:
			outfile.write('{0}:\n'.format(folder))
			outfile.write('\t@$(MAKE) -C {0}/{1}\n'.format(launchpad_name,folder))
			outfile.write('\n\n')















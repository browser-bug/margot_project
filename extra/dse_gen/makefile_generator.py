import os

common_profile_name = 'profile.margot_dse'

def generate_configuration_makefile( configuration, percentage ):
	with open(os.path.join(configuration.cwd, 'Makefile'), 'w') as outfile:
		outfile.write('{0}: exec\n'.format(common_profile_name))
		outfile.write('\t@echo "[{0}%] EXEC {1}"\n'.format(percentage, configuration.name))
		outfile.write('\t@./exec {0} > stdout.txt 2> stderr.txt\n'.format(' '.join(configuration.flags)))
		outfile.write('\t@cp {0} {1}'.format(configuration.log_file, common_profile_name))
	return os.path.join(configuration.cwd, common_profile_name)



def generate_global_makefile(doe, out_path, py_ops_generator, dest_flags):

	# create the dependencies
	dependencies_log_file = []
	dependencies_folder = []
	for c in doe.plan:
		log_file_path = os.path.join(c.cwd, common_profile_name)
		common_path = os.path.commonpath([log_file_path, out_path])
		dependencies_log_file.append(os.path.join('.', os.path.relpath(log_file_path, common_path)))
		dependencies_folder.append(os.path.join('.', os.path.relpath(c.cwd, common_path)))


	# write the global makefile
	with open(os.path.join(out_path, 'Makefile'), 'w') as outfile:

		# write the global objective of the dse
		outfile.write('dse: {0}\n'.format(' \\\n     '.join(dependencies_log_file)))
		generator_flags = ' '.join(dest_flags)
		outfile.write('\t@python3 {0} {1}\n'.format(py_ops_generator, generator_flags))

		# TODO the global action to generate the oplist

		outfile.write('\n\n\n')

		# write the rule to compute the dependencies
		for index_dep, dep in enumerate(dependencies_log_file):
			outfile.write('{0}:\n'.format(dep))
			outfile.write('\t@make -C {0}\n'.format(dependencies_folder[index_dep]))
			outfile.write('\n\n')
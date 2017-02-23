from .parse_op import parse_ops_csv
from .model_op_list import OperatingPointListModel
from .model_op import OperatingPointModel
from .op_utils import print_op_list_xml
import os
import xml.etree.ElementTree as ET                               # get the xml parser from python
import re                                                        # regular expressions
import scipy.stats as stats
import sys

def get_namespace(element):
  m = re.match('\{.*\}', element.tag)
  return m.group(0) if m else ''


def explore(parameters, plan, conf):
	"""
	Recursevely build the plan
	"""

	# if we are the last then bail out
	if not parameters:
		plan.append(conf)
		return

	# take first key of the parameters
	param_key = [ x for x in parameters.keys()][0]

	# create a duplicate of the params
	new_params = dict(parameters)

	# delete the current configuration
	del new_params[param_key]


	# loop over the values
	for value in parameters[param_key]:

		# duplicate and add the value to the configuration
		this_conf = list(conf)

		# append the current value
		this_conf.extend([param_key, str(value)])

		# go deeper
		explore(new_params, plan, this_conf)


class Postprocess_extraction:
	"""
	this class is needed to extract the lists of ops that have to be postprocessed
	builds a vector for each knob, that contains all the possible values. 
	then a vector of vectors is build as full factorial of all the knob vectors. 
	this list is the "key", all the op list must be read and the op where the knob have that values is extracted.
	after the extraction of all ops, a new list with all the ops with the same knobs is postprocessed.


	do
		init:
			full factorial generation from dse.xml
			creation of list of files from input folder.


		while not full factorial not exhausted do:
			extract op from all files
			postprocess
			generate ops for the final oplist given the constraints on the input features.
		done

		
		"""


	def __init__(self, container_folder_path, dse_file):
		# ------- Open and parse the XML file
		# open and parse the xml file
		try:
			with open(dse_file, 'r') as xml_file:
				xml_tree = ET.parse(xml_file)
		except ET.ParseError as err:
			lineno, column = err.position
			print('[PARSER ERROR]: {0}!'.format(err))
			sys.exit(-1)
		# get the root element
		xml_root = xml_tree.getroot()

		# get the namespace
		namespace = get_namespace(xml_root)


		# ------- Parse all the parameters


		# loop over the applications
		params = {}
		self.params_flag_translator = {}
		for xml_parameter in xml_root.findall('./{0}parameter'.format(namespace)):

			# get the flag of the parameter
			try:
				flag = xml_parameter.attrib['flag']
			except KeyError:
				print('[PARSER ERROR]: Unknown attribute flag!')
				print('                A tag <parameter> must define the attribute "flag"')
				print('                Actual attribute(s) of the node:')
				print_node_attributes(xml_parameter)
				sys.exit(-1)


			# get the name of the parameter
			try:
				name = xml_parameter.attrib['name']
			except KeyError:
				print('[PARSER ERROR]: Unknown attribute name!')
				print('                A tag <parameter> must define the attribute "name"')
				print('                Actual attribute(s) of the node:')
				print_node_attributes(xml_parameter)
				sys.exit(-1)
			self.params_flag_translator[flag] = name



			# get the type of the parameter
			supported_types = ['int', 'float', 'enum']
			try:
				param_type = xml_parameter.attrib['type']
			except KeyError:
				print('[PARSER ERROR]: Unknown attribute type!')
				print('                A tag <parameter> must define the attribute "type"')
				print('                Actual attribute(s) of the node:')
				print_node_attributes(xml_parameter)
				sys.exit(-1)
			if param_type not in supported_types:
				print('[LOGIC ERROR]:  Unable to handle parameter of type "{0}" for parameter with flag {1}'.format(flag, flag))
				print('                Supported types: {0}'.format(','.join(supported_types)))
				sys.exit(-1)



			# handle int or float types
			if param_type == 'int':

				# get the start value
				try:
					start_value = int(xml_parameter.attrib['start_value'])
				except KeyError:
					print('[PARSER ERROR]: Unknown attribute start_value!')
					print('                A tag <parameter> of type "{0}" must define the attribute "start_value"'.format(param_type))
					print('                Actual attribute(s) of the node:')
					print_node_attributes(xml_parameter)
					sys.exit(-1)
				except ValueError:
					print('[LOGIC ERROR]: Unable to convert the start value!')
					print('               The value "{0}" is not an integer'.format(xml_parameter.attrib['start_value']))
					sys.exit(-1)


				# get the stop value
				try:
					stop_value = int(xml_parameter.attrib['stop_value'])
				except KeyError:
					print('[PARSER ERROR]: Unknown attribute stop_value!')
					print('                A tag <parameter> of type "{0}" must define the attribute "stop_value"'.format(param_type))
					print('                Actual attribute(s) of the node:')
					print_node_attributes(xml_parameter)
					sys.exit(-1)
				except ValueError:
					print('[LOGIC ERROR]: Unable to convert the stop value!')
					print('               The value "{0}" is not an integer'.format(xml_parameter.attrib['stop_value']))
					sys.exit(-1)


				# get the stop value
				try:
					step_value = int(xml_parameter.attrib['step'])
				except KeyError:
					step_value = 1
				except ValueError:
					step_value = 1


				# add the values
				values = []
				index = start_value
				while( index <= stop_value):
					values.append(index)
					index += step_value
				params[flag] = values


			elif param_type == 'float':


				# get the start value
				try:
					start_value = float(xml_parameter.attrib['start_value'])
				except KeyError:
					print('[PARSER ERROR]: Unknown attribute start_value!')
					print('                A tag <parameter> of type "{0}" must define the attribute "start_value"'.format(param_type))
					print('                Actual attribute(s) of the node:')
					print_node_attributes(xml_parameter)
					sys.exit(-1)
				except ValueError:
					print('[LOGIC ERROR]: Unable to convert the start value!')
					print('               The value "{0}" is not a float'.format(xml_parameter.attrib['start_value']))
					sys.exit(-1)


				# get the stop value
				try:
					stop_value = float(xml_parameter.attrib['stop_value'])
				except KeyError:
					print('[PARSER ERROR]: Unknown attribute stop_value!')
					print('                A tag <parameter> of type "{0}" must define the attribute "stop_value"'.format(param_type))
					print('                Actual attribute(s) of the node:')
					print_node_attributes(xml_parameter)
					sys.exit(-1)
				except ValueError:
					print('[LOGIC ERROR]: Unable to convert the stop value!')
					print('               The value "{0}" is not a float'.format(xml_parameter.attrib['stop_value']))
					sys.exit(-1)


				# get the stop value
				try:
					step_value = int(xml_parameter.attrib['step'])
				except KeyError:
					step_value = 1
				except ValueError:
					step_value = 1


				# compute the values
				values = []
				index = start_value
				while( index <= stop_value):
					values.append(index)
					index += step_value

				# add the values
				params[flag] = values



			elif param_type == 'enum':


				# get the possible values
				try:
					values = xml_parameter.attrib['values']
				except KeyError:
					print('[PARSER ERROR]: Unknown attribute values!')
					print('                A tag <parameter> of type "{0}" must define the attribute "values"'.format(param_type))
					print('                Actual attribute(s) of the node:')
					print_node_attributes(xml_parameter)
					sys.exit(-1)


				# add the values
				params[flag] = [x for x in values.split(' ') if x ]
#		print(params)
#		print(self.params_flag_translator)

		self.plan = []
		explore(params,self.plan, [])
#		print (self.plan)


#		print (container_folder_path)
#		print ("^^^^^^^^")
		self.knob_design_space=[]
		self.folder_path = container_folder_path
		self.file_list = [f for f in os.listdir(self.folder_path) if os.path.isfile(os.path.join(self.folder_path, f))]
				


	def postprocess(self, thresholds):
		oplist = OperatingPointListModel()
		for item in self.plan:
			op_list_to_postprocess = []
			for filename in self.file_list:

				temp_oplist = parse_ops_csv(os.path.join(self.folder_path, filename),',')
				if temp_oplist == None:
					print ("oplist not found for file:")
					print (os.path.join(self.folder_path, filename))
					sys.exit(-1)
				temp_knob_map={}
				for i in range (0, len(item)/2):
					temp_knob_map[self.params_flag_translator[item[2*i]]]=item[2*i+1]

				op=temp_oplist.get_op(temp_knob_map)
				op_list_to_postprocess.append(op)
#			print (item)
#			for point in op_list_to_postprocess:
#				print (point)

			#post process here!!!
		#	thresholds = [0.002, 0.0015, 0.001, 0.005, 0.01]
			real_thresh = sorted (thresholds)
			#print (real_thresh)
			sorted_list = sorted(op_list_to_postprocess, key=lambda op:op.metrics['unpredictability'] )
			final_point_lists = {}
			for thresh in real_thresh:
			#	print ("working with thresh ",thresh, "for item", item)
#tudent_list = sorted(student_list, key=lambda student: student.mark)
				wip_list = []
				for point in sorted_list:
					tmp =[x.metrics['error'] for x in wip_list ]
					tmp.append(point.metrics['error'])
					if stats.percentileofscore(tmp, thresh,'weak') <= 95:
				#		print(stats.percentileofscore(tmp, thresh,'weak'), '         ', thresh)
						break;
					wip_list.append(point)
				if wip_list:
					final_point_lists[thresh]=wip_list
					if wip_list[-1] == sorted_list[-1]:
						break

			#print (final_point_lists)
			for point in final_point_lists:
#				print ("output points:" )
#				print (point)
				if final_point_lists[point]:
					#create an op that will be in the output oplist.
					#must have the max unpredictability as knob, the target error as metric and
					#all the other metrics have to be averaged
					op = OperatingPointModel()
					for knob in final_point_lists[point][0].knobs:
						op.knobs[knob]=final_point_lists[point][0].knobs[knob]
					for metric in final_point_lists[point][0].metrics:
						op.metrics[metric]=0

					for real_point in sorted_list:
						op.add(real_point)
					for metric in op.metrics.keys():
						op.avg(len (sorted_list), metric)
					op.knobs['unpredictability'] = final_point_lists[point][-1].metrics['unpredictability']
					del op.metrics['unpredictability']
					op.metrics['error']=point
					oplist.ops.append(op)
		

		print_op_list_xml(oplist)


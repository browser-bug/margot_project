from . import parse_op
from . import model_op_list
from . import model_op
from . import dse_workspace as ws
from . import dse_application as app
from . import dse_generate_ops
from . import dse_doe
from .op_utils import print_op_list_xml
import os
import math
import scipy.stats as stats

def equal_dicts(d1, d2, ignore_keys):

	ignored = set(ignore_keys)
	#print (ignored)
	for k1, v1 in d1.items():
		if k1 not in ignored and (k1 not in d2 or d2[k1] != v1):
			#print("equal_dicts, k1 not in ignored and doesnt pass check: ",k1,v1," and d2[k1]is: ", d2[k1])
			#print (type (d1[k1]), type (d2[k1]))
			return False
		for k2, v2 in d2.items():
			if k2 not in ignored and k2 not in d1:
				#print (k2, "is not in d1 or in ignored")
				return False
	#print ("equal_dicts: PASSED")
	return True


class Postprocessor:
	"""
	This class contains the building of the structure for the postprocessing, given the dse xml file.
	Then contains a function to postprocess MonteCarlo simulations. 
	Other actual postprocessing functions can be added.
	The init function prepares all the file to be read for the actual postprocessing, parsing the dse xml
	and navigating the folder structure created with the dse.
	A new command from the margot_cli is needed for every 
	"""
	file_to_process_list = []
	def __init__(self, dse_file, path_workspace_directory, doe_strategy):
		"""
		Opens the xml dse, and recreates the Workspace
		"""
		#create app data structure
		self.my_application = app.Application(dse_file)
		self.doe_strategy = doe_strategy

		#re-create the flags
		app_flags = dse_generate_ops.generate_application_flags(self.my_application)

		#setup the workspace
		my_ws = ws.Workspace(path_workspace_directory,"dummy") #executable not needed. to be tested actually.

		for index_folder_ID in self.my_application.flags.keys():
			self.file_to_process_list.append(os.path.join(my_ws.working_root,my_ws.launchpard_dir_name,str(index_folder_ID),my_ws.outfile_name))

#		print (self.file_to_process_list)


	def postprocessMCS(self,thresholds, aggregated_metric, threshold_metric, aggregation_knob, confidence):
		oplist = model_op_list.OperatingPointListModel()
		op_map_to_postprocess = {}
		firstlist = parse_op.parse_ops_xml (self.file_to_process_list[0])
		oplist.translator =firstlist.translator
		oplist.reverse_translator = firstlist.reverse_translator
		
		my_doe_plan = dse_doe.DoE(self.doe_strategy, self.my_application, 0)
		block_name = firstlist.name
		doe_plan_real = []

		for doe_conf in my_doe_plan.plan:
			del(doe_conf.knob_map[aggregation_knob])
			flag = True
			for dic in doe_plan_real: 
				if doe_conf.knob_map == dic:
					flag = False
					break
			if flag:
				doe_plan_real.append(doe_conf.knob_map)



	#for doe_conf in my_doe_plan.plan:#handles the fact that all the knobs must be the same.
		op_map_to_postprocess = {}
		#print (doe_conf.knob_map)
		for filename in self.file_to_process_list:#get all the different oplists
			temp_oplist = parse_op.parse_ops_xml(filename)
			if temp_oplist == None:
				print ("oplist not found for file:")
				print (filename)
				sys.exit(-1)
			if (temp_oplist.name != block_name):
				print ("DEFENSIVE PROGRAMMING: oplist related to different blocks in the same dse. This should not happen")
				sys.exit(-1)

			#get only the op with all the knobs identical values to the doe
			for op in temp_oplist.ops:
				#flag = True
				#for knob_name in doe_conf.knob_map.keys():
				#	if ((knob_name != aggregation_knob) and (str(doe_conf.knob_map[knob_name]) !=str(op.knobs[knob_name]))):
				#		flag = False
				#		break
				#if (flag):
				op.metrics[threshold_metric]=op.metrics[threshold_metric]*int(confidence)
				op_map_to_postprocess.setdefault(op.knobs[aggregation_knob],[]).append(op)
		
		
		for entry in op_map_to_postprocess:
			op_map_to_postprocess[entry] = sorted (op_map_to_postprocess[entry], key = lambda op:op.metrics[aggregated_metric])

		#print (op_map_to_postprocess)
		#print ("DEBUG:" )
		#for entry in op_map_to_postprocess:
			#for op in op_map_to_postprocess[entry]:
			#		print (op)
			#print ("finished entry", entry)


		
		#perform the actual post processing and insert the found operative points in the final oplist
		#init
		final_point_lists = {}
		final_metric_lists = {}
		prv_line = []
		key_map = {}
		for i in range (0, len(op_map_to_postprocess.keys())):
			prv_line.append(0)
			key_map[list(op_map_to_postprocess.keys())[i]]=i

		for thresh in sorted (thresholds):
			#print ("threshold considered is: ",thresh)
			prv = 0
			for point_list in sorted (op_map_to_postprocess):
				#print ("number of samples is: ",point_list, " while error thresh is: ", thresh)
				starting_aggr = max (prv, prv_line[key_map[point_list]])
				#print ("starting aggr value is: ", starting_aggr, "obtained from prv: ",prv, " and prv line: ", prv_line[key_map[point_list]])
				op_tmp_list = []
				metric_tmp_list = []
				debug_unpr_list = []
				broken_from_inner = False
				#print ("befor iteration: prv is: ",prv, "and the prv_line is: ",prv_line)
				#print ("length of op_map_to_postprocess is", len (op_map_to_postprocess[point_list]))
				for point in op_map_to_postprocess[point_list]:
					try :
						point.metrics[aggregated_metric] = float(point.metrics[aggregated_metric])
						point.metrics[threshold_metric] = float(point.metrics[threshold_metric])
					except ValueError as e:
						print('[ERROR] Unable to convert the aggregated metric or the threshold metric to a float value')
						sys.exit(-1)
					if point.metrics[aggregated_metric] >= starting_aggr:
						op_tmp_list.append(point)
						metric_tmp_list.append(point.metrics[threshold_metric])
						debug_unpr_list.append(point.metrics[aggregated_metric])
						#print ("added")
					if len(op_tmp_list) >= 50:
						#test, if fail exit from inner loop
						#print ("testing: ", metric_tmp_list)
						if stats.percentileofscore(metric_tmp_list, thresh,'weak') <= 95:
							broken_from_inner = True
							#remove last 1% of values in array
							num_to_remove_float = len (op_tmp_list)*0.01
							num_to_remove = int(math.ceil( num_to_remove_float))
							for i in range (0, num_to_remove):
								op_tmp_list.pop()
							break
				#print (op_tmp_list)
				#print ("broken out from loop, op_tmp_list size is: ",len(op_tmp_list))
				if len (op_tmp_list) > 50:
					#ok, create op and update tmp data structures
					final_point_lists.setdefault(point_list,[]).append(op_tmp_list)
					final_metric_lists.setdefault(point_list,[]).append(thresh)  ##same order!
					if (broken_from_inner):
						prv = op_tmp_list[-1].metrics[aggregated_metric]
					#else:
					#	print (metric_tmp_list)
					#	print (debug_unpr_list)
					prv_line [key_map[point_list]]=op_tmp_list[-1].metrics[aggregated_metric]
		for doe_plan in doe_plan_real:
			#print (doe_plan_real)
			for point in final_point_lists:
				#print (doe_plan)
				#print (point)
				#print (final_point_lists)
				#print (len (final_point_lists[point]))
				for i in range (0, len( final_point_lists[point])):
					#print ("DEBUG: point is: ", point)
					#create an op that will be in the output oplist.
					#must have the max unpredictability as knob, the target error as metric and
					#all the other metrics have to be averaged to the correct doeplan.
					
					##########################################################
					#							 NTS:						 #
					# every doe point has to have (MUST) a point in the new  #
					# oplist, so if no input groups have been given in the   #
					# dse to cluster with, it will fail here since at least  #
					# one of the list of correct points will be empty because#
					# the point cannot be in more than one clustered lists   #
					##########################################################
					out_op = model_op.OperatingPointModel()
					for knob in final_point_lists[point][i][0].knobs:
						if knob != aggregation_knob:
							out_op.knobs[knob]=doe_plan[knob]       #final_point_lists[point][i][0].knobs[knob]
					out_op.knobs[aggregation_knob]=int(point)
					for metric in final_point_lists[point][i][0].metrics:
						out_op.metrics[metric]=0
					#print (final_point_lists[point][i])
					#for op in final_point_lists[point][i]:
					#	print (op)
					#print (doe_plan)
					list_of_correct_points = [x for x in final_point_lists[point][i] if equal_dicts(doe_plan, oplist.get_op_knobs_as_string(x),[aggregation_knob])]
					if len (list_of_correct_points) == 0:
						print ("ERROR IN DATASET: no point are found in cluster that have the doe value",doe_plan)
						sys.exit(1)
					#print (list_of_correct_points)
					for op in list_of_correct_points:
						#print (op)
						out_op.add(op)
					for metric in out_op.metrics.keys():
						#print ("++++****")
						#print ("adding metric: ",metric)
						#print (out_op)
						out_op.avg(len (list_of_correct_points), metric)
					#print ("out")
					out_op.knobs[aggregated_metric] = list_of_correct_points[-1].metrics[aggregated_metric]
					del out_op.metrics[aggregated_metric]
					out_op.metrics[threshold_metric] = final_metric_lists[point][i]
					oplist.ops.append(out_op)
				#print ("out inner")
			#clean temp, and proceed with next doe in plan.
			
		oplist.name = block_name
		print_op_list_xml(oplist)



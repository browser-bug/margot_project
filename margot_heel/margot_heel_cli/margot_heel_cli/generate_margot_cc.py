import os                                # for creating the path
import errno                             # for checking before creating a path


from .generate_utility import generate_start_monitor_signature
from .generate_utility import generate_stop_monitor_signature
from .generate_utility import generate_update_signature



from .model_data_feature import DataFeatureModel



what_translator = {
  "AVERAGE"  : "average",
  "STDDEV"   : "standard_deviation",
  "MAX"      : "max",
  "MIN"      : "min",
}

cfun_feature_translator = {
  "GE"  : "margot::FeatureComparison::GREATER_OR_EQUAL",
  "LE"  : "margot::FeatureComparison::LESS_OR_EQUAL",
  "-"   : "margot::FeatureComparison::DONT_CARE"
}



def generate_block_body( block_model, op_lists, cc ):
  """
  Generates the per block code
  """

  # write the begin of the namespace
  cc.write('\n\n\tnamespace {0} {{\n'.format(block_model.block_name))

  # write all the monitors
  if block_model.monitor_models:

    # open the monitor namespace
    cc.write('\n\t\tnamespace monitor {\n')

    # loop over the monitor
    error_monitor = {}
    thereIsErrorMonitor = False
    for monitor_model in block_model.monitor_models:
      cc.write('\t\t\t{0} {1};\n'.format(monitor_model.monitor_class, monitor_model.monitor_name))

      # look for the error monitor and prepare the information about it
      if monitor_model.type.upper() == "ERROR":
        thereIsErrorMonitor = True
        if monitor_model.frequency == "periodic":
          error_monitor[monitor_model.monitor_name] = monitor_model.period
        else:
          error_monitor[monitor_model.monitor_name] = monitor_model.frequency

    uniqueErrorMonitor = False
    errorType = None
    errorPeriod = None
    errorPeriodicDictionary = None

    if thereIsErrorMonitor:
      # if we have just one error monitor
      if (len(error_monitor) == 1):
        uniqueErrorMonitor = True
        # if the error monitor is periodic
        if ((error_monitor[error_monitor.keys()[0]]).isdigit()):
          errorType = "periodic"
          errorPeriod = error_monitor[error_monitor.keys()[0]] # save the frequency
        else:
          errorType = error_monitor[error_monitor.keys()[0]]
      # if all the error monitors are "training" then treat it as just a single "training" monitor
      elif (len(error_monitor) == list(error_monitor.values()).count("training")):
        errorType = error_monitor[error_monitor.keys()[0]]
        uniqueErrorMonitor = True
      # if there exists even only just one error monitor which is "always" then treat it as a single "always" monitor
      elif list(error_monitor.values()).count("always") >= 1:
        errorType = "always"
        uniqueErrorMonitor = True
      else:
        # remove all the "training" error monitors and just keep the periodic ones (those are the only one which can still remain after these checks)
        errorPeriodicDictionary = {k:v for k,v in error_monitor.items() if v != 'training'}
        # now we only have "periodic" monitors
        tempPeriod = errorPeriodicDictionary[errorPeriodicDictionary.keys()[0]]
        uniqueErrorMonitor = True
        for k,v in errorPeriodicDictionary.items():
          if not (tempPeriod == v):
            uniqueErrorMonitor = False
        if uniqueErrorMonitor == True: # all the error monitors are periodic with the same period
          errorType = "periodic"
          errorPeriod = errorPeriodicDictionary[errorPeriodicDictionary.keys()[0]] # save the frequency
        # we are in the case in which all the error monitors are periodic but with different periods
        else:
          errorType = "periodic"

    # close the monitor namespace
    cc.write('\t\t} // namespace monitor\n')

  # write all the goals
  if block_model.goal_models:

    # open the goal namespace
    cc.write('\n\t\tnamespace goal {\n')

    # loop over the goals
    for goal_model in block_model.goal_models:

      #try to figure out the type of the goal
      goal_type = 'long double'
      if goal_model.metric_name_ref:
        for metric_model in block_model.metrics:
          if metric_model.name == goal_model.metric_name_ref:
            goal_type = metric_model.type
      if goal_model.parameter_name_ref:
        for knob_model in block_model.software_knobs:
          if knob_model.name == goal_model.parameter_name_ref:
            goal_type = knob_model.var_type

      # write its declaration
      cc.write('\t\t\t{1} {0};\n'.format(goal_model.name, goal_model.get_c_goal_type(goal_type)))

    # close the goal namespace
    cc.write('\t\t} // namespace goal\n')

  # write the observed data features variables (if any)
  if (block_model.features):

    # open the data feature namespace
    cc.write('\n\t\tnamespace features {\n')

    # loop over the features
    for feature in block_model.features:
      cc.write('\t\t\t{1} {0};\n'.format(feature.name, feature.type))

    # close the feature namespace
    cc.write('\t\t} // namespace features\n')

  # open the software knobs namespace
  cc.write('\n\t\tnamespace knobs {\n')

  # loop over the knobs
  for knob_model in block_model.software_knobs:
    cc.write('\t\t\t{1} {0};\n'.format(knob_model.var_name, knob_model.var_type))

  # close the software knobs namespace
  cc.write('\t\t} // namespace knobs\n')

  # write the manager (check if we have data feature)
  if (block_model.metrics and block_model.software_knobs ):
      if (block_model.features):

        # get the names of data feature, in alphabetical order
        names = sorted([ x.name for x in block_model.features ])

        # set the type of the asrtm
        features_types = set([x.type for x in block_model.features])
        features_types_indexes = [ DataFeatureModel.available_var_types.index(x) for x in features_types]
        features_type = DataFeatureModel.available_var_types[max(features_types_indexes)]

        # loop over the data feature fields to compose the type
        features_cf = []
        for name in names:
          # get the corresponding data feature comparison function
          feature_cf = [ x.cf for x in block_model.features if x.name == name][0]
          features_cf.append(cfun_feature_translator[feature_cf])

        # print the new type of the asrtm
        cc.write('\n\n\t\tDataAwareAsrtm< Asrtm< MyOperatingPoint >, {0}, margot::FeatureDistanceType::{1}, {2} > manager;\n\n\n'.format(features_type, block_model.feature_distance, ', '.join(features_cf)))
      else:
        # write the manager
        cc.write('\n\n\t\tAsrtm< MyOperatingPoint > manager;\n\n\n')

  # write the logger
  cc.write('\n\t\t#ifdef MARGOT_LOG_FILE\n')
  cc.write('\t\tLogger file_logger;\n')
  cc.write('\t\t#endif // MARGOT_LOG_FILE\n\n\n')

  # write all the exposed variables from the monitors
  for monitor_model in block_model.monitor_models:
    for exposed_var_what in monitor_model.exposed_metrics:
      cc.write('\n\t\t{1}::statistical_type {0};\n'.format(monitor_model.exposed_metrics[exposed_var_what], monitor_model.monitor_class))


  # write the helper variables for the error management
  if thereIsErrorMonitor:
    cc.write('\n\t\tbool expectingErrorReturn = false;\n')
    # write the counters for the period(s)
    # if there is a single period
    if ((errorType == "periodic") and uniqueErrorMonitor):
      cc.write('\n\t\tint errorIterationCounter = 0;\n')
    # if there are multiple periods append the name of the corresponding monitor
    elif (errorType == "periodic"):
      for elem in errorPeriodicDictionary.keys():
        cc.write('\n\t\tint errorIterationCounter_{0} = 0;\n'.format(elem))


  # write the datasets structs (if the detasets are provided) with built-in check if the training and production datasets have the same data structure
  if len(block_model.datasets_model)==2:
      thereIsString = False
      for input_data_model in block_model.datasets_model[0].input_data_models:
          if (input_data_model.type == "string"):
              thereIsString = True

      #write the two vectors of struct already filled with data
      for dataset_model in block_model.datasets_model:
          if dataset_model.type == "training":
             structName = "std::vector<dataset> trainingStruct = {"
             cc.write('\n\t\t{0}\n'.format(structName))
             for x in range(len(dataset_model.input_data_models[0].values)):
                 cc.write('\t\t\t{')
                 for y in range(len(dataset_model.input_data_models)):
                     # manage the final comma and the quotes for the strings
                     if (dataset_model.input_data_models[y].type == "string"):
                         cc.write('"{0}"'.format(dataset_model.input_data_models[y].values[x]))
                     else:
                         cc.write('{0}'.format(dataset_model.input_data_models[y].values[x]))
                     if not (y == (len(dataset_model.input_data_models)-1)):
                         cc.write(', ')
                 if (x == (len(dataset_model.input_data_models[0].values)-1)):
                     cc.write('}\n')
                 else:
                     cc.write('},\n')
             cc.write('\t\t};\n')
          else:
             structName = "std::vector<dataset> productionStruct = {"
             cc.write('\n\t\t{0}\n'.format(structName))
             for x in range(len(dataset_model.input_data_models[0].values)):
                 cc.write('\t\t\t{')
                 for y in range(len(dataset_model.input_data_models)):
                     # manage the final comma and the quotes for the strings
                     if (dataset_model.input_data_models[y].type == "string"):
                         cc.write('"{0}"'.format(dataset_model.input_data_models[y].values[x]))
                     else:
                         cc.write('{0}'.format(dataset_model.input_data_models[y].values[x]))
                     if not (y == (len(dataset_model.input_data_models)-1)):
                         cc.write(', ')
                 if (x == (len(dataset_model.input_data_models[0].values)-1)):
                     cc.write('}\n')
                 else:
                     cc.write('},\n')
             cc.write('\t\t};\n')

      #generate iterators for datasets
      cc.write("\n\t\tstd::vector<dataset>::iterator itTrain = trainingStruct.begin();")
      cc.write("\n\t\tstd::vector<dataset>::iterator itProd = productionStruct.begin();")

      #generate other helper variables
      cc.write("\n\n\t\tbool work_to_do = true;")
      cc.write("\n\t\tbool lastDatasetPassedWasTraining;")

      #generate the helper data structures
      cc.write("\n\n\t\tdataset currentDataset;")
      cc.write("\n\t\tdataset currentDatasetTemp;")
      #if thereIsString:
      cc.write("\n\t\tdatasetC_{0} currentDatasetC;".format(block_model.block_name))


      #write the getDataset function
      cc.write("\n\n\t\tdataset get_dataset(){")
      cc.write("\n\t\t\tif (!manager.has_model()){")
      cc.write("\n\t\t\t\tlastDatasetPassedWasTraining = true;")
      cc.write("\n\t\t\t\tcurrentDataset = *itTrain;")
      cc.write("\n\t\t\t\treturn currentDataset;")
      cc.write("\n\t\t\t} else {")
      cc.write("\n\t\t\t\tlastDatasetPassedWasTraining = false;")
      cc.write("\n\t\t\t\tcurrentDataset = *itProd;")
      cc.write("\n\t\t\t\treturn currentDataset;")
      cc.write("\n\t\t\t}")
      cc.write("\n\t\t}")

      #TODO: write the getDatasetForC
      cc.write("\n\n\t\tdatasetC_{0} get_datasetC()".format(block_model.block_name))
      cc.write('\n\t\t{')
      cc.write("\n\t\t\tcurrentDatasetTemp = get_dataset();")
      for x in range(len(dataset_model.input_data_models)):
          if (dataset_model.input_data_models[x].type == "string"):
              cc.write("\n\t\t\tcurrentDatasetC.{0} = const_cast<char*>(currentDatasetTemp.{0}.c_str());".format(dataset_model.input_data_models[x].name))
          else:
              cc.write("\n\t\t\tcurrentDatasetC.{0} = currentDatasetTemp.{0};".format(dataset_model.input_data_models[x].name))
      cc.write("\n\t\t\treturn currentDatasetC;")
      cc.write("\n\t\t}")

      #write the next function
      cc.write("\n\n\t\tvoid next(){")
      cc.write("\n\t\t\tif (lastDatasetPassedWasTraining){")
      cc.write("\n\t\t\t\tif ((itTrain+1) != trainingStruct.end()){")
      cc.write("\n\t\t\t\t\titTrain++;")
      cc.write("\n\t\t\t\t} else {")
      cc.write("\n\t\t\t\t\titTrain = trainingStruct.begin();")
      cc.write("\n\t\t\t\t}")
      cc.write("\n\t\t\t} else {")
      cc.write("\n\t\t\t\tif ((itProd+1) != productionStruct.end()){")
      cc.write("\n\t\t\t\t\titProd++;")
      cc.write("\n\t\t\t\t} else {")
      cc.write("\n\t\t\t\t\twork_to_do = false;")
      cc.write("\n\t\t\t\t}")
      cc.write("\n\t\t\t}")
      cc.write("\n\t\t}")

      #write the toDo function
      cc.write("\n\n\t\tbool to_do(){")
      cc.write("\n\t\t\treturn work_to_do;")
      cc.write("\n\t\t}")


      #write the dataset_type_switch function
      cc.write("\n\n\t\t//TODO:check if we need to use the 'has_model()' or the 'in_design_space_exploration' function")
      cc.write("\n\t\tbool dataset_type_switch(){")
      cc.write("\n\t\t\tif ((lastDatasetPassedWasTraining && (manager.has_model())) || (!lastDatasetPassedWasTraining && (!manager.has_model()))){")
      cc.write("\n\t\t\t\treturn true;")
      cc.write("\n\t\t\t} else {")
      cc.write("\n\t\t\t\treturn false;")
      cc.write("\n\t\t\t}")
      cc.write("\n\t\t}")


      #TODO: update header.hpp with new methods



  # write the update function
  cc.write('\n\n\t\tbool {0}\n'.format(generate_update_signature(block_model)))
  cc.write('\t\t{\n')

  # check if we should handle the data features
  if (block_model.metrics and block_model.software_knobs ):
      if block_model.features:

        # write the statements that assign the data features to the global variable
        feature_names = []
        for feature in block_model.features:
          cc.write('\t\t\tfeatures::{0} = {0};\n'.format(feature.name))
          feature_names.append(feature.name)

        # write the statement that select the correct asrtm
        feature_string = '{{{{{0}}}}}'.format(', '.join(feature_names))
        cc.write('\t\t\tmanager.select_feature_cluster({0});\n'.format(feature_string))


      cc.write('\t\t\tif (!manager.is_application_knowledge_empty())\n')
      cc.write('\t\t\t{\n')
      cc.write('\t\t\t\tmanager.find_best_configuration();\n')
      cc.write('\t\t\t\tbool conf_changed = false;\n')
      cc.write('\t\t\t\tconst auto new_conf = manager.get_best_configuration(&conf_changed);\n')
      cc.write('\t\t\t\tif (conf_changed)\n')
      cc.write('\t\t\t\t{\n')
      param_list = ['{0}& {1}'.format(x.var_type, x.var_name) for x in block_model.software_knobs]
      for knob in block_model.software_knobs:
        cc.write('\t\t\t\t\t{0} = new_conf.get_mean<static_cast<std::size_t>(Knob::{1})>();\n'.format(knob.var_name, knob.name.upper()))
      cc.write('\t\t\t\t}\n')
      for knob in block_model.software_knobs:
        cc.write('\t\t\t\tknobs::{0} = {0};\n'.format(knob.var_name))
      cc.write('\t\t\t\treturn conf_changed;\n')
      cc.write('\t\t\t}\n')
      for knob in block_model.software_knobs:
        cc.write('\t\t\tknobs::{0} = {0};\n'.format(knob.var_name))
      cc.write('\t\t\treturn false;\n')
  else:
      cc.write('\t\t\treturn false;\n')
  cc.write('\t\t}\n')

  # write the start_monitor function
  cc.write('\n\n\t\tvoid {0}\n'.format(generate_start_monitor_signature(block_model)))
  cc.write('\t\t{\n')
  for monitor_model in block_model.monitor_models:
    if monitor_model.start_method:
      cc.write('\t\t\tmonitor::{0}.{1}('.format(monitor_model.monitor_name, monitor_model.start_method))
      possible_arguments = [x.var_name for x in monitor_model.start_parameters if x.var_name]
      possible_arguments.extend([str(x.param_value) for x in monitor_model.start_parameters if x.param_value])
      cc.write(','.join(possible_arguments))
      cc.write(');\n')
  cc.write('\t\t}\n')


  # write the stop_monitor function
  cc.write('\n\n\t\tvoid {0}\n'.format(generate_stop_monitor_signature(block_model)))
  cc.write('\t\t{\n')

  # generate the code that stops the monitors
  # version if there is (at least) a monitor which can be disables (just error monitor by now)
  if thereIsErrorMonitor:
    # write the c++ if-else runtime logic to manage if the monitor is currently enabled/disables
    # if we do NOT expect the monitor because it is currently disabled
    cc.write('\t\t\tif (!(expectingErrorReturn)) {\n')
    for monitor_model in block_model.monitor_models:
      # if the current monitor in analysis is the disabled one then discard it. The value that the user provides will be discarded too.
      if monitor_model.type.upper() == "ERROR":
        continue
      if monitor_model.stop_method:
        cc.write('\t\t\t\tmonitor::{0}.{1}('.format(monitor_model.monitor_name, monitor_model.stop_method))
        possible_arguments = [x.var_name for x in monitor_model.stop_parameters if x.var_name]
        possible_arguments.extend([str(x.param_value) for x in monitor_model.stop_parameters if x.param_value])
        cc.write(','.join(possible_arguments))
        cc.write(');\n')

    # if we have agora, we need to generate also the code that sends the information
    if not block_model.agora_model is None:
        cc.write('\n')

        # get the list of knobs, features and metrics
        knobs = sorted([x for x in block_model.agora_model.knobs_values])
        features = sorted([x for x in block_model.agora_model.features_values])
        metrics = sorted([x for x in block_model.agora_model.metrics_monitors])
        # get the list of metrics enabled for the beholder analysis
        beholder_metrics = sorted([x for x in block_model.agora_model.beholder_metrics])

        # actually understand whether or not the user specified some metrics for the beholder
        beholderMetricsExplicit = False
        # if there is at least one element in the list of metrics then the user specified the metrics for the beholder explicitly
        if len(beholder_metrics) > 0:
          beholderMetricsExplicit = True
          # check the consistency of the beholder-metrics. Check if these are a subset of the metrics
          if not all(a in metrics for a in beholder_metrics):
              raise Exception("The metrics specified for the beholder are not correct. Check the names!")


        # start to compose the list of terms to send to
        knob_terms = []
        for knob in knobs:
            for knob_model in block_model.software_knobs:
                if knob_model.name == knob:
                    knob_var_name = knob_model.var_name
                    break
            knob_terms.append('std::to_string(knobs::{0})'.format(knob_var_name))
        knob_string = ' + "," + '.join(knob_terms)
        feature_terms = []
        for feature in features:
            feature_terms.append('std::to_string(features::{0})'.format(feature))
        feature_string = ' + "," + '.join(feature_terms)
        metric_terms = []
        # prepare a list also for the currently enabled metric names
        metric_name_list = []
        # prepare a list also for the currently enabled metric predictions
        metric_prediction_list = []
        # structures as the ones above but for the case in which the user explicitly specified the beholder metrics
        if beholderMetricsExplicit:
          beholder_metric_terms = []
          # prepare a list also for the currently enabled metric names
          beholder_metric_name_list = []
          # prepare a list also for the currently enabled metric predictions
          beholder_metric_prediction_list = []
          # prepare a list also for all the beholder-enabled metric predictions
          all_beholder_metric_prediction_list = []

        # NB:  the following "for-else-break" structure is to "continue" the outer loop when we meet the disabled monitor.
        #      This is needed NOT to enque the disabled monitor's value in the message
        # NB2: we need to "connect" the metric to the related "monitors", in particular we need to disable the metrics
        #      which are related to the currently disabled monitors.
        #      This is needed because we will save into Cassandra the metrics and not the monitors' values,
        #      but we disable the monitors in the XML and not the metrics.
        #      In the "disabled" case we need to disable both the monitor and the metric.
        #NB3:  The "double" cycle over the metric and the monitors is needed because the metrics's monitor (below "related_monitor")
        #      is just a string and it is not the monitor object of which we can query the type.
        for metric in metrics: # for each metric
            related_monitor = block_model.agora_model.metrics_monitors[metric] # get the monitor to which the metric is referring to
            for monitor_model in block_model.monitor_models: # for each monitor
              if monitor_model.monitor_name == related_monitor: # if we match the metric's monitor (string) with the monitor's name (string) in the cycle we are analyzing
                # we query the monitor object's type and if this is the disabled monitor then discard it and go to the next one.
                if monitor_model.type.upper() == "ERROR":
                  break
            else:
              # if the current metric is one of the enabled ones then append the monitor's value to the list of metric values
              metric_terms.append('std::to_string(monitor::{0}.last())'.format(related_monitor))
              # if the current metric is one of the enabled ones then append the metric's name to the list of metric names
              metric_name_list.append('std::string("{0}")'.format(metric))
              # if the current metric is one of the enabled ones then append the metric's prediction from the model to the list of metric predictions
              metric_prediction_list.append('std::to_string({0}::manager.get_mean<OperatingPointSegments::METRICS, static_cast<std::size_t>({0}::Metric::{1})>())'.format(block_model.block_name, metric.upper()))
              # put in the beholder lists just the intersection of the currently enabled metrics and the beholder metrics
              if beholderMetricsExplicit:
                if metric in beholder_metrics:
                  # if the current metric is one of the enabled ones then append the monitor's value to the list of metric values
                  beholder_metric_terms.append('std::to_string(monitor::{0}.last())'.format(related_monitor))
                  # if the current metric is one of the enabled ones then append the metric's name to the list of metric names
                  beholder_metric_name_list.append('std::string("{0}")'.format(metric))
                  # if the current metric is one of the enabled ones then append the metric's prediction from the model to the list of metric predictions
                  beholder_metric_prediction_list.append('std::to_string({0}::manager.get_mean<OperatingPointSegments::METRICS, static_cast<std::size_t>({0}::Metric::{1})>())'.format(block_model.block_name, metric.upper()))

        metric_string = ' + "," + '.join(metric_terms)
        # use the same technique of join also for the currently enabled metric names
        metric_names = ' + "," + '.join(metric_name_list)

        if beholderMetricsExplicit:
            # build the list of all the beholder-enabled metrics (not just the currently available ones, but all those specified by the user)
            for metric in beholder_metrics:
                # if the current metric is one of the enabled ones then append the metric's name to the list of metric names
                all_beholder_metric_prediction_list.append('std::string("{0}")'.format(metric))
            beholder_metric_string = ' + "," + '.join(beholder_metric_terms)
            # use the same technique of join also for the currently enabled metric names
            beholder_metric_names = ' + "," + '.join(beholder_metric_name_list)

        # at run-time condition: if the client has the model then send the observation
        # to both the agora and beholder application handlers
        cc.write('\t\t\t\tif (manager.has_model()) {\n')

        # use the same technique of join also for the currently enabled metric predictions
        metric_predictions = ' + "," + '.join(metric_prediction_list)

        if beholderMetricsExplicit:
            # use the same technique of join also for the currently enabled metric predictions
            beholder_metric_predictions = ' + "," + '.join(beholder_metric_prediction_list)
            # enqueue the list of all the metrics that the user enabled for the beholder analysis
            all_beholder_metric_names = ' + "," + '.join(all_beholder_metric_prediction_list)

        #build the message for the beholder according to the case in which the beholder metrics are explicitly specified or not
        if beholderMetricsExplicit:
            # if the user enabled some explict metrics for the beholder, then enqueue send just the currently enabled metrics observations and the list of all the beholder-enabled metrics
            # so that the beholder always knows how many metrics it currently receives out of all those it could potentially receive
            send_beholder_string = ' + " " + '.join([beholder_metric_string, beholder_metric_predictions, beholder_metric_names, all_beholder_metric_names])
        else:
            # if the user does not explicitly states the metrics enabled for the beholder than it is assumed that all of them are
            send_beholder_string = ' + " " + '.join([metric_string, metric_predictions, metric_names])

        # append also the metric names to the message that will be sent to agora
        if feature_terms:
            send_string = ' + " " + '.join([knob_string, feature_string, metric_string, metric_predictions, metric_names])
        else:
            send_string = ' + " " + '.join([knob_string, metric_string, metric_predictions, metric_names])
        cc.write('\t\t\t\t\tmanager.send_observation({0}, {1});\n'.format(send_string,send_beholder_string))

        # else send the observation to just the agora remote application handler
        # send a "null" in place of the predictions so that the trace estimates are left empty
        cc.write('\t\t\t\t} else {\n')

        # use the same technique of join also for the currently enabled metric predictions
        metric_predictions = "\"null\""

        # append also the metric names to the message that will be sent to agora
        if feature_terms:
            send_string = ' + " " + '.join([knob_string, feature_string, metric_string, metric_predictions, metric_names])
        else:
            send_string = ' + " " + '.join([knob_string, metric_string, metric_predictions, metric_names])
        cc.write('\t\t\t\t\tmanager.send_observation({0});\n'.format(send_string))

        # end of the run-time condition
        cc.write('\t\t\t\t}\n')

    #if we are in training and we expect a return value for all the monitors, then we behave as if all the monitors are always enabled (else below)
    cc.write('\t\t\t} else {\n')
    for monitor_model in block_model.monitor_models:
      if monitor_model.stop_method:
        cc.write('\t\t\t\tmonitor::{0}.{1}('.format(monitor_model.monitor_name, monitor_model.stop_method))
        possible_arguments = [x.var_name for x in monitor_model.stop_parameters if x.var_name]
        possible_arguments.extend([str(x.param_value) for x in monitor_model.stop_parameters if x.param_value])
        cc.write(','.join(possible_arguments))
        cc.write(');\n')

    # if we have agora, we need to generate also the code that sends the information
    if not block_model.agora_model is None:
        cc.write('\n')

        # get the list of knobs, features and metrics
        knobs = sorted([x for x in block_model.agora_model.knobs_values])
        features = sorted([x for x in block_model.agora_model.features_values])
        metrics = sorted([x for x in block_model.agora_model.metrics_monitors])
        # get the list of metrics enabled for the beholder analysis
        beholder_metrics = sorted([x for x in block_model.agora_model.beholder_metrics])

        # actually understand whether or not the user specified some metrics for the beholder
        beholderMetricsExplicit = False
        # if there is at least one element in the list of metrics then the user specified the metrics for the beholder explicitly
        if len(beholder_metrics) > 0:
          beholderMetricsExplicit = True

        # start to compose the list of terms to send to
        knob_terms = []
        for knob in knobs:
            for knob_model in block_model.software_knobs:
                if knob_model.name == knob:
                    knob_var_name = knob_model.var_name
                    break
            knob_terms.append('std::to_string(knobs::{0})'.format(knob_var_name))
        knob_string = ' + "," + '.join(knob_terms)
        feature_terms = []
        for feature in features:
            feature_terms.append('std::to_string(features::{0})'.format(feature))
        feature_string = ' + "," + '.join(feature_terms)
        metric_terms = []
        # prepare a list also for the currently enabled metric names
        metric_name_list = []
        # prepare a list also for the currently enabled metric predictions
        metric_prediction_list = []
        # structures as the ones above but for the case in which the user explicitly specified the beholder metrics
        if beholderMetricsExplicit:
          beholder_metric_terms = []
          # prepare a list also for the currently enabled metric names
          beholder_metric_name_list = []
          # prepare a list also for the currently enabled metric predictions
          beholder_metric_prediction_list = []
          # prepare a list also for all the beholder-enabled metric predictions
          all_beholder_metric_prediction_list = []
        for metric in metrics:
            related_monitor = block_model.agora_model.metrics_monitors[metric]
            metric_terms.append('std::to_string(monitor::{0}.last())'.format(related_monitor))
            # if the current metric is one of the enabled ones then append the metric's name to the list of metric names
            metric_name_list.append('std::string("{0}")'.format(metric))
            # if the current metric is one of the enabled ones then append the metric's prediction from the model to the list of metric predictions
            metric_prediction_list.append('std::to_string({0}::manager.get_mean<OperatingPointSegments::METRICS, static_cast<std::size_t>({0}::Metric::{1})>())'.format(block_model.block_name, metric.upper()))
            # put in the beholder lists just the intersection of the currently enabled metrics and the beholder metrics
            if beholderMetricsExplicit:
              if metric in beholder_metrics:
                # if the current metric is one of the enabled ones then append the monitor's value to the list of metric values
                beholder_metric_terms.append('std::to_string(monitor::{0}.last())'.format(related_monitor))
                # if the current metric is one of the enabled ones then append the metric's name to the list of metric names
                beholder_metric_name_list.append('std::string("{0}")'.format(metric))
                # if the current metric is one of the enabled ones then append the metric's prediction from the model to the list of metric predictions
                beholder_metric_prediction_list.append('std::to_string({0}::manager.get_mean<OperatingPointSegments::METRICS, static_cast<std::size_t>({0}::Metric::{1})>())'.format(block_model.block_name, metric.upper()))
        metric_string = ' + "," + '.join(metric_terms)

        # use the same technique of join also for the currently enabled metric names
        metric_names = ' + "," + '.join(metric_name_list)

        if beholderMetricsExplicit:
            # build the list of all the beholder-enabled metrics (not just the currently available ones, but all those specified by the user)
            for metric in beholder_metrics:
                # if the current metric is one of the enabled ones then append the metric's name to the list of metric names
                all_beholder_metric_prediction_list.append('std::string("{0}")'.format(metric))
            beholder_metric_string = ' + "," + '.join(beholder_metric_terms)
            # use the same technique of join also for the currently enabled metric names
            beholder_metric_names = ' + "," + '.join(beholder_metric_name_list)

        # at run-time condition: if the client has the model then send the observation
        # to both the agora and beholder application handlers
        cc.write('\t\t\t\tif (manager.has_model()) {\n')

        # use the same technique of join also for the currently enabled metric predictions
        metric_predictions = ' + "," + '.join(metric_prediction_list)

        if beholderMetricsExplicit:
            # use the same technique of join also for the currently enabled metric predictions
            beholder_metric_predictions = ' + "," + '.join(beholder_metric_prediction_list)# enqueue the list of all the metrics that the user enabled for the beholder analysis
            all_beholder_metric_names = ' + "," + '.join(all_beholder_metric_prediction_list)

        #build the message for the beholder according to the case in which the beholder metrics are explicitly specified or not
        if beholderMetricsExplicit:
            # if the user enabled some explict metrics for the beholder, then enqueue send just the currently enabled metrics observations and the list of all the beholder-enabled metrics
            # so that the beholder always knows how many metrics it currently receives out of all those it could potentially receive
            send_beholder_string = ' + " " + '.join([beholder_metric_string, beholder_metric_predictions, beholder_metric_names, all_beholder_metric_names])
        else:
            # if the user does not explicitly states the metrics enabled for the beholder than it is assumed that all of them are
            send_beholder_string = ' + " " + '.join([metric_string, metric_predictions, metric_names])

        if feature_terms:
            send_string = ' + " " + '.join([knob_string, feature_string, metric_string, metric_predictions])
        else:
            send_string = ' + " " + '.join([knob_string, metric_string, metric_predictions])
        cc.write('\t\t\t\t\tmanager.send_observation({0}, {1});\n'.format(send_string,send_beholder_string))

        # else send the observation to just the agora remote application handler
        # send a "null" in place of the predictions so that the trace estimates are left empty
        cc.write('\t\t\t\t} else {\n')

        # use the same technique of join also for the currently enabled metric predictions
        metric_predictions = "\"null\""

        # append also the metric names to the message that will be sent to agora
        if feature_terms:
            send_string = ' + " " + '.join([knob_string, feature_string, metric_string, metric_predictions])
        else:
            send_string = ' + " " + '.join([knob_string, metric_string, metric_predictions])
        cc.write('\t\t\t\t\tmanager.send_observation({0});\n'.format(send_string))

        # end of the run-time condition
        cc.write('\t\t\t\t}\n')

    cc.write('\t\t\t}')
  # version if all the monitors are always enabled
  else:
    for monitor_model in block_model.monitor_models:
      if monitor_model.stop_method:
        cc.write('\t\t\tmonitor::{0}.{1}('.format(monitor_model.monitor_name, monitor_model.stop_method))
        possible_arguments = [x.var_name for x in monitor_model.stop_parameters if x.var_name]
        possible_arguments.extend([str(x.param_value) for x in monitor_model.stop_parameters if x.param_value])
        cc.write(','.join(possible_arguments))
        cc.write(');\n')

    # if we have agora, we need to generate also the code that sends the information
    if not block_model.agora_model is None:
        cc.write('\n')

        # get the list of knobs, features and metrics
        knobs = sorted([x for x in block_model.agora_model.knobs_values])
        features = sorted([x for x in block_model.agora_model.features_values])
        metrics = sorted([x for x in block_model.agora_model.metrics_monitors])
        # get the list of metrics enabled for the beholder analysis
        beholder_metrics = sorted([x for x in block_model.agora_model.beholder_metrics])

        # actually understand whether or not the user specified some metrics for the beholder
        beholderMetricsExplicit = False
        # if there is at least one element in the list of metrics then the user specified the metrics for the beholder explicitly
        if len(beholder_metrics) > 0:
          beholderMetricsExplicit = True

        # start to compose the list of terms to send to
        knob_terms = []
        for knob in knobs:
            for knob_model in block_model.software_knobs:
                if knob_model.name == knob:
                    knob_var_name = knob_model.var_name
                    break
            knob_terms.append('std::to_string(knobs::{0})'.format(knob_var_name))
        knob_string = ' + "," + '.join(knob_terms)
        feature_terms = []
        for feature in features:
            feature_terms.append('std::to_string(features::{0})'.format(feature))
        feature_string = ' + "," + '.join(feature_terms)
        metric_terms = []
        # prepare a list also for the currently enabled metric names
        metric_name_list = []
        # prepare a list also for the currently enabled metric predictions
        metric_prediction_list = []
        # structures as the ones above but for the case in which the user explicitly specified the beholder metrics
        if beholderMetricsExplicit:
          beholder_metric_terms = []
          # prepare a list also for the currently enabled metric names
          beholder_metric_name_list = []
          # prepare a list also for the currently enabled metric predictions
          beholder_metric_prediction_list = []
          # prepare a list also for all the beholder-enabled metric predictions
          all_beholder_metric_prediction_list = []
        for metric in metrics:
            related_monitor = block_model.agora_model.metrics_monitors[metric]
            metric_terms.append('std::to_string(monitor::{0}.last())'.format(related_monitor))
            # if the current metric is one of the enabled ones then append the metric's name to the list of metric names
            metric_name_list.append('std::string("{0}")'.format(metric))
            # if the current metric is one of the enabled ones then append the metric's prediction from the model to the list of metric predictions
            metric_prediction_list.append('std::to_string({0}::manager.get_mean<OperatingPointSegments::METRICS, static_cast<std::size_t>({0}::Metric::{1})>())'.format(block_model.block_name, metric.upper()))
            # put in the beholder lists just the intersection of the currently enabled metrics and the beholder metrics
            if beholderMetricsExplicit:
              if metric in beholder_metrics:
                # if the current metric is one of the enabled ones then append the monitor's value to the list of metric values
                beholder_metric_terms.append('std::to_string(monitor::{0}.last())'.format(related_monitor))
                # if the current metric is one of the enabled ones then append the metric's name to the list of metric names
                beholder_metric_name_list.append('std::string("{0}")'.format(metric))
                # if the current metric is one of the enabled ones then append the metric's prediction from the model to the list of metric predictions
                beholder_metric_prediction_list.append('std::to_string({0}::manager.get_mean<OperatingPointSegments::METRICS, static_cast<std::size_t>({0}::Metric::{1})>())'.format(block_model.block_name, metric.upper()))
        metric_string = ' + "," + '.join(metric_terms)

        # use the same technique of join also for the currently enabled metric names
        metric_names = ' + "," + '.join(metric_name_list)

        if beholderMetricsExplicit:
            # build the list of all the beholder-enabled metrics (not just the currently available ones, but all those specified by the user)
            for metric in beholder_metrics:
                # if the current metric is one of the enabled ones then append the metric's name to the list of metric names
                all_beholder_metric_prediction_list.append('std::string("{0}")'.format(metric))
            beholder_metric_string = ' + "," + '.join(beholder_metric_terms)
            # use the same technique of join also for the currently enabled metric names
            beholder_metric_names = ' + "," + '.join(beholder_metric_name_list)

        # at run-time condition: if the client has the model then send the observation
        # to both the agora and beholder application handlers
        cc.write('\t\t\tif (manager.has_model()) {\n')

        # use the same technique of join also for the currently enabled metric predictions
        metric_predictions = ' + "," + '.join(metric_prediction_list)

        if beholderMetricsExplicit:
            # use the same technique of join also for the currently enabled metric predictions
            beholder_metric_predictions = ' + "," + '.join(beholder_metric_prediction_list)# enqueue the list of all the metrics that the user enabled for the beholder analysis
            all_beholder_metric_names = ' + "," + '.join(all_beholder_metric_prediction_list)

        #build the message for the beholder according to the case in which the beholder metrics are explicitly specified or not
        if beholderMetricsExplicit:
            # if the user enabled some explict metrics for the beholder, then enqueue send just the currently enabled metrics observations and the list of all the beholder-enabled metrics
            # so that the beholder always knows how many metrics it currently receives out of all those it could potentially receive
            send_beholder_string = ' + " " + '.join([beholder_metric_string, beholder_metric_predictions, beholder_metric_names, all_beholder_metric_names])
        else:
            # if the user does not explicitly states the metrics enabled for the beholder than it is assumed that all of them are
            send_beholder_string = ' + " " + '.join([metric_string, metric_predictions, metric_names])

        if feature_terms:
            send_string = ' + " " + '.join([knob_string, feature_string, metric_string, metric_predictions])
        else:
            send_string = ' + " " + '.join([knob_string, metric_string, metric_predictions])
        cc.write('\t\t\t\tmanager.send_observation({0}, {1});\n'.format(send_string,send_beholder_string))

        # else send the observation to just the agora remote application handler
        # send a "null" in place of the predictions so that the trace estimates are left empty
        cc.write('\t\t\t\t} else {\n')

        # use the same technique of join also for the currently enabled metric predictions
        metric_predictions = "\"null\""

        # append also the metric names to the message that will be sent to agora
        if feature_terms:
            send_string = ' + " " + '.join([knob_string, feature_string, metric_string, metric_predictions])
        else:
            send_string = ' + " " + '.join([knob_string, metric_string, metric_predictions])
        cc.write('\t\t\t\tmanager.send_observation({0});\n'.format(send_string))

        # end of the run-time condition
        cc.write('\t\t\t}\n')

  cc.write('\n')
  cc.write('\t\t}\n')


  # write the "has_model()" function
  cc.write('\n\n\t\tbool has_model()\n')
  cc.write('\t\t{\n')
  cc.write('\t\t\t return manager.has_model();\n')
  cc.write('\t\t}\n')

  #write the compute_error function
  cc.write("\n\n\t\tbool compute_error()")
  cc.write("\n\t\t{")
  if not thereIsErrorMonitor:
    cc.write("\n\t\t\treturn false;")
  else:
    if errorType == "always":
      cc.write("\n\t\t\texpectingErrorReturn = true;")
      cc.write("\n\t\t\treturn true;")
    if errorType == "training":
      cc.write("\n\t\t\texpectingErrorReturn = (!(manager.has_model()) || (manager.are_metrics_on()));")
      cc.write("\n\t\t\treturn !(manager.has_model());")
    if errorType == "periodic":
      # if the error monitor is unique or all the error monitors have the same period
      if uniqueErrorMonitor:
        cc.write("\n\t\t\tif (manager.has_model()) {")
        cc.write("\n\t\t\t\tif (errorIterationCounter == {0})".format(errorPeriod))
        cc.write("\n\t\t\t\t{")
        cc.write("\n\t\t\t\t\texpectingErrorReturn = true;")
        cc.write("\n\t\t\t\t\terrorIterationCounter = 0;")
        cc.write("\n\t\t\t\t\treturn true;")
        # check whether the beholder ordered to enable the metrics
        cc.write("\n\t\t\t\t} else {")
        cc.write("\n\t\t\t\t\texpectingErrorReturn = manager.are_metrics_on();")
        cc.write("\n\t\t\t\t\terrorIterationCounter++;")
        cc.write("\n\t\t\t\t\treturn manager.are_metrics_on();")
        cc.write("\n\t\t\t\t}")
        # we are in training and the metrics are always enabled
        cc.write("\n\t\t\t} else {")
        cc.write("\n\t\t\t\texpectingErrorReturn = true;")
        cc.write("\n\t\t\t\treturn true;")
        cc.write("\n\t\t\t}")
      # if the error monitors have different periods
      else:
        cc.write("\n\t\t\tif (manager.has_model()) {")
        # compose the line for the C++ "if" clause
        cPeriodMatch = ""
        for k, v in errorPeriodicDictionary.items():
          cPeriodMatch = ("{0}errorIterationCounter_{1} == {2} || ".format(cPeriodMatch,k,v))
        cPeriodMatch = cPeriodMatch[:-4]
        cc.write("\n\t\t\t\tif ({0})".format(cPeriodMatch))
        cc.write("\n\t\t\t\t{")
        cc.write("\n\t\t\t\t\texpectingErrorReturn = true;")
        cc.write("\n\t\t\t\t\tstd::vector<std::string> tempCounterIncreased;")
        # reset all the period counters for the monitors which have triggered the error computation in this run
        for k, v in errorPeriodicDictionary.items():
          cc.write("\n\t\t\t\t\tif (errorIterationCounter_{0} == {1})".format(k,v))
          cc.write("\n\t\t\t\t\t{")
          cc.write("\n\t\t\t\t\t\terrorIterationCounter_{0} = 0;".format(k))
          cc.write("\n\t\t\t\t\t\ttempCounterIncreased.push_back(\"errorIterationCounter_{0}\");".format(k))
          cc.write("\n\t\t\t\t\t}")
        # increase the period counter for all the monitors which have NOT triggered the error computation in this run
        for k, v in errorPeriodicDictionary.items():
          cc.write("\n\t\t\t\t\tif (!(std::find(tempCounterIncreased.begin(), tempCounterIncreased.end(), \"errorIterationCounter_{0}\") != tempCounterIncreased.end()))".format(k,v))
          cc.write("\n\t\t\t\t\t{")
          cc.write("\n\t\t\t\t\t\terrorIterationCounter_{0}++;".format(k))
          cc.write("\n\t\t\t\t\t}")
        cc.write("\n\t\t\t\t\treturn true;")
        # check whether the beholder ordered to enable the metrics
        cc.write("\n\t\t\t\t} else {")
        cc.write("\n\t\t\t\t\texpectingErrorReturn = manager.are_metrics_on();")
        # increase the period counter for all the monitors
        for k, v in errorPeriodicDictionary.items():
          cPeriodIncrease = ("errorIterationCounter_{0}++".format(k))
          cc.write("\n\t\t\t\t\t{0};".format(cPeriodIncrease))
        cc.write("\n\t\t\t\t\treturn manager.are_metrics_on();")
        cc.write("\n\t\t\t\t}")
        # we are in training and the metrics are always enabled
        cc.write("\n\t\t\t} else {")
        cc.write("\n\t\t\t\texpectingErrorReturn = true;")
        cc.write("\n\t\t\t\treturn true;")
        cc.write("\n\t\t\t}")
  cc.write("\n\t\t}")


  # write the log function
  cc.write('\n\n\t\tvoid log( void )\n')
  cc.write('\t\t{\n')

  # at first update the exposed variables
  exposed_variables = []
  for monitor_model in block_model.monitor_models:
    for exposed_metric_name in monitor_model.exposed_metrics:
      exposed_variables.append(monitor_model.exposed_metrics[exposed_metric_name])
      cc.write('\t\t\t{0} = monitor::{1}.{2}();\n'.format(monitor_model.exposed_metrics[exposed_metric_name], monitor_model.monitor_name, what_translator[exposed_metric_name.upper()]))
  cc.write('\n')


  # keep track of what we are printing (for the cout)
  what_we_are_printing = []

  # compute the getters for the data features
  cluster_feature_printer = []
  data_feature_names = sorted([ x.name for x in block_model.features ])
  for index, feature_name in enumerate(data_feature_names):
    what_we_are_printing.append('Cluster {0}'.format(feature_name))
    cluster_feature_printer.append('{0}::manager.get_selected_feature<{1}>()'.format(block_model.block_name, index))


  # compute the getters for software knobs
  software_knobs_printers = []
  for knob in block_model.software_knobs:
    what_we_are_printing.append('Knob {0}'.format(knob.name))
    if block_model.block_name in op_lists:
      if knob.name in op_lists[block_model.block_name][list(op_lists[block_model.block_name].keys())[0]].translator:
        software_knobs_printers.append('{0}::knob_{2}_int2str[static_cast<int>({0}::manager.get_mean<OperatingPointSegments::SOFTWARE_KNOBS, static_cast<std::size_t>({0}::Knob::{1})>()]'.format(block_model.block_name, knob.name.upper(), knob.name.lower()))
        continue
    software_knobs_printers.append('{0}::manager.get_mean<OperatingPointSegments::SOFTWARE_KNOBS, static_cast<std::size_t>({0}::Knob::{1})>()'.format(block_model.block_name, knob.name.upper()))


  # compute the getters for the metrics
  metrics_printers = []
  for metric in block_model.metrics:
    what_we_are_printing.append('Expected {0}'.format(metric.name))
    metrics_printers.append('{0}::manager.get_mean<OperatingPointSegments::METRICS, static_cast<std::size_t>({0}::Metric::{1})>()'.format(block_model.block_name, metric.name.upper()))


  # compute the getters for the goals
  goal_printers = []
  for goal in block_model.goal_models:
    what_we_are_printing.append('Goal {0}'.format(goal.name))
    goal_printers.append('{0}::goal::{1}.get()'.format(block_model.block_name, goal.name))

  # compute the getters for the monitors
  monitor_printers = []
  for monitor_model in block_model.monitor_models:
    for exposed_var_what in monitor_model.exposed_metrics:
      what_we_are_printing.append('{0}'.format(monitor_model.exposed_metrics[exposed_var_what]))
      monitor_printers.append('{0}::{1}'.format(block_model.block_name, monitor_model.exposed_metrics[exposed_var_what]))


  # compute the getters for the observed data features
  data_feature_printer = []
  data_feature_names = sorted([ x.name for x in block_model.features ])
  for feature_name in data_feature_names:
    what_we_are_printing.append('Input {0}'.format(feature_name))
    data_feature_printer.append('{0}::features::{1}'.format(block_model.block_name, feature_name))


  # ------- actually compose the log functin on file

  cc.write('\n\t\t\t#ifdef MARGOT_LOG_FILE\n')
  if (block_model.metrics and block_model.software_knobs ):
      cc.write('\t\t\tif (!(manager.is_application_knowledge_empty() || manager.in_design_space_exploration()))\n')
      cc.write('\t\t\t{\n')


      # if we have ops it's easythen print the stuff
      things_to_print = list(cluster_feature_printer)
      things_to_print.extend(software_knobs_printers)
      things_to_print.extend(metrics_printers)
      things_to_print.extend(goal_printers)
      things_to_print.extend(monitor_printers)
      things_to_print.extend(data_feature_printer)
      string_to_print = ',\n\t\t\t\t\t'.join(things_to_print)
      cc.write('\t\t\t\tfile_logger.write(')
      cc.write('{0});\n'.format(string_to_print))


      cc.write('\t\t\t}\n')
      cc.write('\t\t\telse\n')
      cc.write('\t\t\t{\n')

      # if we have no ops, we must made up the expected stuff
      software_knobs_printers_alternative = ['{1}::knobs::{0}'.format(x.var_name, block_model.block_name) for x in block_model.software_knobs]
      metrics_printers_alternative = ['"N/A"' for x in metrics_printers]
      cluster_feature_printer_alternative = ['"N/A"' for x in cluster_feature_printer]

      # then print the stuff
      things_to_print = list(cluster_feature_printer_alternative)
      things_to_print.extend(software_knobs_printers_alternative)
      things_to_print.extend(metrics_printers_alternative)
      things_to_print.extend(goal_printers)
      things_to_print.extend(monitor_printers)
      things_to_print.extend(data_feature_printer)
      string_to_print = ',\n\t\t\t\t\t'.join(things_to_print)
      cc.write('\t\t\t\tfile_logger.write(')
      cc.write('{0});\n'.format(string_to_print))



      cc.write('\t\t\t}\n')
  else:
      things_to_print = list(goal_printers)
      things_to_print.extend(monitor_printers)
      things_to_print.extend(data_feature_printer)
      string_to_print = ',\n\t\t\t\t\t'.join(things_to_print)
      cc.write('\t\t\t\tfile_logger.write(')
      cc.write('{0});\n'.format(string_to_print))
  cc.write('\t\t\t#endif // MARGOT_LOG_FILE\n')



  # ------- actually compose the log functin on ostream


  cc.write('\n\t\t\t#ifdef MARGOT_LOG_STDOUT\n')

  if (block_model.metrics and block_model.software_knobs ):
      cc.write('\t\t\tif (!(manager.is_application_knowledge_empty() || manager.in_design_space_exploration()))\n')
      cc.write('\t\t\t{\n')

      # then print the stuff
      things_to_print = list(cluster_feature_printer)
      things_to_print.extend(software_knobs_printers)
      things_to_print.extend(metrics_printers)
      things_to_print.extend(goal_printers)
      things_to_print.extend(monitor_printers)
      things_to_print.extend(data_feature_printer)

      # set the endl from arguments
      endl_indexes = [len(cluster_feature_printer)]
      endl_indexes.append( endl_indexes[-1] + len(software_knobs_printers))
      endl_indexes.append( endl_indexes[-1] + len(metrics_printers))
      endl_indexes.append( endl_indexes[-1] + len(goal_printers))
      endl_indexes.append( endl_indexes[-1] + len(monitor_printers))
      endl_indexes.append( endl_indexes[-1] + len(data_feature_printer))

      # write the print statement
      composed_elements = []
      for index, content in enumerate(things_to_print):
        if index in endl_indexes:
          composed_elements.append('std::endl')
        string = '"[ {0} = " << {1} << "] "'.format(what_we_are_printing[index], content)
        composed_elements.append(string)
      string_to_print = '\n\t\t\t\t\t<< '.join(composed_elements)
      cc.write('\t\t\t\tstd::cout <<')
      cc.write('{0} << std::endl;\n'.format(string_to_print))


      cc.write('\t\t\t}\n')
      cc.write('\t\t\telse\n')
      cc.write('\t\t\t{\n')

      # then print the stuff
      things_to_print = list(cluster_feature_printer_alternative)
      things_to_print.extend(software_knobs_printers_alternative)
      things_to_print.extend(metrics_printers_alternative)
      things_to_print.extend(goal_printers)
      things_to_print.extend(monitor_printers)
      things_to_print.extend(data_feature_printer)

      # set the endl from arguments
      endl_indexes = [len(cluster_feature_printer_alternative)]
      endl_indexes.append( endl_indexes[-1] + len(software_knobs_printers_alternative))
      endl_indexes.append( endl_indexes[-1] + len(metrics_printers))
      endl_indexes.append( endl_indexes[-1] + len(goal_printers))
      endl_indexes.append( endl_indexes[-1] + len(monitor_printers))
      endl_indexes.append( endl_indexes[-1] + len(data_feature_printer))

      # write the print statement
      composed_elements = []
      for index, content in enumerate(things_to_print):
        if index in endl_indexes:
          composed_elements.append('std::endl')
        string = '"[ {0} = " << {1} << "] "'.format(what_we_are_printing[index], content)
        composed_elements.append(string)
      string_to_print = '\n\t\t\t\t\t<< '.join(composed_elements)
      cc.write('\t\t\t\tstd::cout <<')
      cc.write('{0} << std::endl;\n'.format(string_to_print))


      cc.write('\t\t\t}\n')
  else:
      things_to_print = list(goal_printers)
      things_to_print.extend(monitor_printers)
      things_to_print.extend(data_feature_printer)

      # set the endl from arguments
      endl_indexes = [len(goal_printers)]
      endl_indexes.append( endl_indexes[-1] + len(monitor_printers))
      endl_indexes.append( endl_indexes[-1] + len(data_feature_printer))

      # write the print statement
      composed_elements = []
      for index, content in enumerate(things_to_print):
        if index in endl_indexes:
          composed_elements.append('std::endl')
        string = '"[ {0} = " << {1} << "] "'.format(what_we_are_printing[index], content)
        composed_elements.append(string)
      string_to_print = '\n\t\t\t\t\t<< '.join(composed_elements)
      cc.write('\t\t\t\tstd::cout <<')
      cc.write('{0} << std::endl;\n'.format(string_to_print))
  cc.write('\t\t\t#endif // MARGOT_LOG_STDOUT\n')


  # close the log method
  cc.write('\t\t}\n')

  # write the end of the namespace
  cc.write('\n\t}} // namespace {0}\n'.format(block_model.block_name))









def generate_margot_cc( block_models, op_lists, output_folder ):
  """
  This function generates the actual margot source files
  """


  # open the output file
  with open(os.path.join(output_folder, 'margot.cc'), 'w') as cc:

    # write the include
    cc.write('#include <string>\n')
    cc.write('#include "margot.hpp"\n')
    cc.write('#include "margot_op_struct.hpp"\n')
    cc.write('#ifdef MARGOT_LOG_STDOUT\n')
    cc.write('#include <iostream>\n')
    cc.write('#endif // MARGOT_LOG_STDOUT\n')
    cc.write('#ifdef MARGOT_LOG_FILE\n')
    cc.write('#include "margot_logger.hpp"\n')
    cc.write('#endif // MARGOT_LOG_FILE\n')

    # if we have the "pause" element in agora (and we have agora) we need to add these includes
    # loop over the blocks
    for block_name in block_models:
      # get the reference to the block
      block_model = block_models[block_name]
      if not block_model.agora_model is None:
        for block_name in block_models:
          if not block_models[block_name].agora_model.pause_polling == "":
              cc.write('#include <chrono>\n')
              cc.write('#include <thread>\n')
              break

    # if we have the wrapper we need to add these includes
    for block_name in block_models:
      if len(block_models[block_name].datasets_model)==2:
          cc.write('#include <vector>\n')
          cc.write('#include <iterator>\n')
          break

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


    # generate the code block-specific code
    for block_name in block_models:
      generate_block_body(block_models[block_name], op_lists, cc)




    # ----------- generate the init function signature
    cc.write('\n\n\tvoid init( ')

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

    cc.write('{0} )\n'.format(signature))



    # ----------- generate the init function body
    cc.write('\t{\n')

    # loop over the blocks
    for block_name in block_models:

      # writing a preamble
      cc.write('\n\n\t\t// --------- Initializing the block "{0}"\n'.format(block_name.upper()))

      # get the reference to the block
      block_model = block_models[block_name]

      # generate the monitor initialization
      for monitor_model in block_model.monitor_models:
        cc.write('\t\t{0}::monitor::{1} = {2}('.format(block_name,monitor_model.monitor_name, monitor_model.monitor_class))
        creation_params = [x.var_name for x in monitor_model.creation_parameters if x.var_name]
        creation_params.extend( [str(x.param_value) for x in monitor_model.creation_parameters if x.param_value] )
        cc.write('{0});\n'.format(', '.join(creation_params)))

      # set the goal to their actual values
      for goal_model in block_model.goal_models:
        cc.write('\t\t{0}::goal::{1}.set({2});\n'.format(block_name, goal_model.name, goal_model.value))



      # check if we have any Operating Points
      if block_name in op_lists:
        cc.write('\n\t\t// Adding the application knowledge\n')

        # get all the op list for data cluster
        op_list_ids = sorted(op_lists[block_name].keys())

        # loop over them to create Operating Points
        for index, op_list_feature_id in enumerate(op_list_ids):

          # make a tuple out of the id
          data_feature = '{{{{{0}}}}}'.format(op_list_feature_id.replace('|',', '))

          # check if we actually have to create a data cluster
          if block_model.features:

            # insert the call that creates the feature cluster
            cc.write('\t\t{0}::manager.add_feature_cluster({1});\n'.format(block_name, data_feature))

            # insert the call which selects the feature cluster
            cc.write('\t\t{0}::manager.select_feature_cluster({1});\n'.format(block_name, data_feature))

          # eventually, insert the call which adds the Operating Points
          cc.write('\t\t{0}::manager.add_operating_points({0}::op_list{1});\n'.format(block_name, index))
      else:

        # if we have no Operating Point lists, but we do have data features,
        # we need to create a dummy cluster
        if block_model.features:
            dummy_feature = '{{{{{0}}}}}'.format(','.join(['0' for x in range(len(block_model.features))]))
            cc.write('\t\t{0}::manager.add_feature_cluster({1});\n'.format(block_name, dummy_feature))
            cc.write('\t\t{0}::manager.select_feature_cluster({1});\n'.format(block_name, dummy_feature))
      cc.write('\n')


      # check if we need to insert runtime information providers
      if block_model.field_adaptor_models:
        cc.write('\n\t\t// Adding runtime information provider(s)\n')
        for field_adaptor_model in block_model.field_adaptor_models:

          # check if the adaptor is in the metric
          if field_adaptor_model.metric_name:

            # get the index of the metric (TODO check if we can remove the cast)
            metric_index = 'static_cast<std::size_t>({0}::Metric::{1})'.format(block_name, field_adaptor_model.metric_name.upper())

            # write the instruction to the file
            cc.write('\t\t{0}::manager.add_runtime_knowledge<OperatingPointSegments::METRICS, {1}, {2}>({0}::monitor::{3});\n'.format(
              block_name, metric_index, field_adaptor_model.inertia, field_adaptor_model.monitor_name
              ))

          # check if the adaptor is in the software knob
          if field_adaptor_model.knob_name:

            # get the index of the metric
            knob_index = 'static_cast<std::size_t>({0}::Knob::{1})'.format(block_name, field_adaptor_model.knob_name.upper())

            # write the instruction to the file
            cc.write('\t\t{0}::manager.add_runtime_knowledge<OperatingPointSegments::SOFTWARE_KNOBS, {1}, {2}({0}::monitor::{3});\n'.format(
              block_name, metric_index, field_adaptor_model.inertia, field_adaptor_model.monitor_name
              ))


      # writing a preamble
      cc.write('\n\n\t\t// --------- Defining the application requirements for block "{0}"\n'.format(block_name.upper()))

      # loop over the states
      for state_model in block_model.state_models:

        # add & switch to the current state
        cc.write('\n\t\t// Defining the state "{0}"\n'.format(state_model.name))
        cc.write('\t\t{0}::manager.create_new_state("{1}");\n'.format(block_name, state_model.name))
        cc.write('\t\t{0}::manager.change_active_state("{1}");\n'.format(block_name, state_model.name))

        # i'm going to define the rank
        rank_objective = state_model.rank_available_directions[state_model.rank_direction]
        rank_composition = state_model.rank_available_combination_types[state_model.rank_type]

        # get the list of the rank fields
        rank_fields = []
        for rank_field_model in state_model.rank_fields:
          if rank_field_model.metric_name:
            metric_index = 'static_cast<std::size_t>({0}::Metric::{1})'.format(block_name, rank_field_model.metric_name.upper())
            rank_fields.append('OPField<OperatingPointSegments::METRICS,BoundType::LOWER,{0},0>'.format(metric_index))
          if rank_field_model.knob_name:
            knob_index = 'static_cast<std::size_t>({0}::Knob::{1})'.format(block_name, rank_field_model.knob_name.upper())
            rank_fields.append('OPField<OperatingPointSegments::SOFTWARE_KNOBS,BoundType::LOWER,{0},0>'.format(knob_index))

        # get the rank coefs
        rank_coefs = [str(float(x.coefficient)) for x in state_model.rank_fields]
        funcion_coefs = ', '.join(rank_coefs)

        # set the template arguments
        template_args = '{0}, {1}, {2} '.format(
          state_model.rank_available_directions[state_model.rank_direction],
          state_model.rank_available_combination_types[state_model.rank_type],
          ', '.join(rank_fields)
          )

        # print the template args
        if rank_fields:
            cc.write('\n\t\t // Defining the application rank\n')
            cc.write('\t\t{0}::manager.set_rank<{1}>({2});\n'.format(block_name, template_args, funcion_coefs))

        # loop over the constraints
        for constraint_model in state_model.constraint_list:
          if constraint_model.target_knob:
            cc.write('\t\t{0}::manager.add_constraint<OperatingPointSegments::SOFTWARE_KNOBS,{1},{2}>({0}::goal::{3}, {4});\n'.format(
            block_name,
            'static_cast<std::size_t>({0}::Knob::{1})'.format(block_name, constraint_model.target_knob.upper()),
            constraint_model.confidence,
            constraint_model.goal_ref,
            constraint_model.priority
            ))
          if constraint_model.target_metric:
            cc.write('\t\t{0}::manager.add_constraint<OperatingPointSegments::METRICS,{1},{2}>({0}::goal::{3}, {4});\n'.format(
            block_name,
            'static_cast<std::size_t>({0}::Metric::{1})'.format(block_name, constraint_model.target_metric.upper()),
            constraint_model.confidence,
            constraint_model.goal_ref,
            constraint_model.priority
            ))


      # switch to the active state
      if block_model.state_models:
        active_state = block_model.state_models[0]
        for state_model in block_model.state_models:
          if state_model.starting:
            active_state = state_model
            break
        cc.write('\n\t\t// Switch to the starting active state\n')
        cc.write('\t\t{0}::manager.change_active_state("{1}");\n'.format(block_name, active_state.name))


      # store the information to print
      cc.write('\n\t\t// Initialize the log file\n')
      cc.write('\t\t#ifdef MARGOT_LOG_FILE\n')
      if (block_model.metrics and block_model.software_knobs ):
          data_feature_names = sorted([x.name for x in block_model.features])
          things_to_print = ['\t\t\t"Cluster_{0}"'.format(x.upper()) for x in data_feature_names]
          things_to_print.extend(['\t\t\t"Knob_{0}"'.format(x.name.upper()) for x in block_model.software_knobs])
          things_to_print.extend(['\t\t\t"Known_Metric_{0}"'.format(x.name.upper()) for x in block_model.metrics])
          things_to_print.extend(['\t\t\t"Goal_{0}"'.format(x.name.upper()) for x in block_model.goal_models])
          for monitor_model in block_model.monitor_models:
            for exposed_var_what in monitor_model.exposed_metrics:
              things_to_print.append('\t\t\t"{1}_{0}"'.format(exposed_var_what, monitor_model.monitor_name.upper()))
          things_to_print.extend(['\t\t\t"Input_{0}"'.format(x.upper()) for x in data_feature_names])
          cc.write('\t\t{0}::file_logger.open("{0}.log", margot::Format::PLAIN,\n{1});\n'.format(block_name, ',\n'.join(things_to_print)))
      else:
          data_feature_names = sorted([x.name for x in block_model.features])
          things_to_print = ['\t\t\t"Goal_{0}"'.format(goal.name.upper()) for x in block_model.goal_models]
          for monitor_model in block_model.monitor_models:
            for exposed_var_what in monitor_model.exposed_metrics:
              things_to_print.append('\t\t\t"{1}_{0}"'.format(exposed_var_what, monitor_model.monitor_name.upper()))
          things_to_print.extend(['\t\t\t"Input_{0}"'.format(x.upper()) for x in data_feature_names])
          cc.write('\t\t{0}::file_logger.open("{0}.log", margot::Format::PLAIN,\n{1});\n'.format(block_name, ',\n'.join(things_to_print)))
      cc.write('\t\t#endif // MARGOT_LOG_FILE\n')


      # if we have the agora application local handler, we have to spawn the support thread
      if not block_model.agora_model is None:

          # get all the knobs, metrics and features of the block
          knob_list = sorted([x.name for x in block_model.software_knobs])
          metric_list = sorted([x.name for x in block_model.metrics])
          feature_list = sorted([x.name for x in block_model.features])

          # define the type of a metric
          metrics_type = []
          for metric_model in block_model.metrics:
            metrics_type.append(metric_model.type)
          if 'int' in metrics_type:
            metric_type_pod = 'float'
          if 'float' in metrics_type:
            metric_type_pod = 'float'
          if 'double' in metrics_type:
            metric_type_pod = 'double'

          # define the type of a software knobs
          knobs_type = []
          for knob_model in block_model.software_knobs:
            knobs_type.append(knob_model.var_type)
          if 'int' in knobs_type:
            knob_type_pod = 'int'
          if 'float' in knobs_type:
            knob_type_pod = 'float'
          if 'double' in knobs_type:
            knob_type_pod = 'double'

          # define the type of a feature
          features_type = []
          for feature_model in block_model.features:
            features_type.append(feature_model.type)
          if 'int' in features_type:
            feature_type_pod = 'int'
          if 'float' in features_type:
            feature_type_pod = 'float'
          if 'double' in features_type:
            feature_type_pod = 'double'

          # compose the descrition of the application
          description_terms = []
          for knob_name in knob_list:
               description_terms.append('knob      {0} {1} {2}'.format(knob_name, knob_type_pod, block_model.agora_model.knobs_values[knob_name]))
          for feature_name in feature_list:
               description_terms.append('feature   {0} {1} {2}'.format(feature_name, feature_type_pod, block_model.agora_model.features_values[feature_name]))
          for metric_name in metric_list:
               description_terms.append('metric    {0} {1} {2}'.format(metric_name, metric_type_pod, block_model.agora_model.metrics_predictors[metric_name]))
          description_terms.append('doe       {0}'.format(block_model.agora_model.doe))
          description_terms.append('num_obser {0}'.format(block_model.agora_model.number_observation))
          description_string = '@'.join(description_terms)

          # generate the argument of the initialization
          app_name = block_model.agora_model.application_name
          broker_url = block_model.agora_model.broker_url
          broker_username = block_model.agora_model.username
          broker_password = block_model.agora_model.password
          broker_ca = block_model.agora_model.broker_ca
          client_cert = block_model.agora_model.client_cert
          client_key = block_model.agora_model.client_key
          mqtt_qos = block_model.agora_model.qos
          parameter_string = '"{0}","{1}","{2}","{3}",{4},"{5}","{6}","{7}","{8}"'.format(app_name,broker_url,broker_username,broker_password,mqtt_qos,description_string,broker_ca,client_cert,client_key )

          # eventually emit the code that starts the agora application local handler
          cc.write('\n\t\t// Start the agora pocal application handler thread\n')
          cc.write('\t\t{0}::manager.start_support_thread<{0}::operating_point_parser_t>({1});\n'.format(block_name, parameter_string))


          #if we have the "pause" element in agora we need to create the busy waiting according to the polling and timeout set by the user
          if not block_model.agora_model.pause_polling == "":
            #without timeout
            if block_model.agora_model.pause_timeout == "-1":
              cc.write('\n\t\t// Busy waiting: wait for agora synchronization')
              cc.write('\n\t\twhile ({0}::manager.is_application_knowledge_empty())'.format(block_name))
              cc.write('\n\t\t{')
              cc.write('\n\t\t\tstd::this_thread::sleep_for(std::chrono::milliseconds({0}));'.format(block_model.agora_model.pause_polling))
              cc.write('\n\t\t}\n')
            else:
              #with timeout
              cc.write('\n\t\t// Busy waiting: wait for agora synchronization')
              cc.write('\n\t\tint timeout = {0};'.format(block_model.agora_model.pause_timeout))
              cc.write('\n\t\twhile ({0}::manager.is_application_knowledge_empty())'.format(block_name))
              cc.write('\n\t\t{')
              cc.write('\n\t\t\tstd::this_thread::sleep_for(std::chrono::milliseconds({0}));'.format(block_model.agora_model.pause_polling))
              cc.write('\n\t\t\ttimeout = timeout - {0};'.format(block_model.agora_model.pause_polling))
              cc.write('\n\t\t\tif (timeout < 0)')
              cc.write('\n\t\t\t{')
              cc.write('\n\t\t\t\tbreak;')
              cc.write('\n\t\t\t}')
              cc.write('\n\t\t}\n')


    cc.write('\t}\n')

    # write some trailer spaces
    cc.write('\n\n')

    # write the margot namespace end
    cc.write('} // namespace margot\n\n')

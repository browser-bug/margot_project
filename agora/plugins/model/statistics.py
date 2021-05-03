import os
import time
import pandas as pd
import numpy as np
from pathlib import Path

def store_stats(metric_name, num_iteration, cv_results, scores, data, target, stats_dir):
    """
    Utility module to store some useful statistics data during learning procedure. The data can be manipulated afterwards by the end-user.

    metric_name : name of the EFP of interest
    num_iteration : number of iterations performed during agora learning process
    cv_results : dictionary containing for each key (estimator name) the cross validation results
    scores : list of the score metrics evaluated during the modelling
    data : pandas dataframe representing the data set (software-knobs and input-features if any)
    target : pandas dataframe representing the target set (EFP of interest for each configuration in the data set)
    stats_dir : path of the directory where files will be stored
    """

    def split_all(path):
        allparts = []
        while True:
            parts = os.path.split(path)
            if parts[0] == path:  # sentinel for absolute paths
                allparts.insert(0, parts[0])
                break
            elif parts[1] == path: # sentinel for relative paths
                allparts.insert(0, parts[1])
                break
            else:
                path = parts[0]
                allparts.insert(0, parts[1])
        return allparts

    # retrieve the application name
    plugin_working_dir = Path.cwd()
    app_name = split_all(plugin_working_dir)[-4]
    stats_directory = Path(stats_dir + app_name)
    # create the subdirectory for the application
    if not stats_directory.exists():
        Path.mkdir(stats_directory)

    # store the data and target tables for the current iteration
    data_path = stats_directory / f"data_itr_{num_iteration}.csv"
    target_path = stats_directory / f"target_{metric_name}_itr_{num_iteration}.csv"
    data.to_csv(data_path, index=False)
    target.to_csv(target_path, index=False)

    # compute the header of the file
    model_stats_path = stats_directory / f"model_{metric_name}_stats.csv"
    if model_stats_path.exists():
        model_stats_df = pd.read_csv(model_stats_path)
    else:
        header = ['timestamp','nr_iter','nr_configs']
        for score in scores:
            header.append(f"{score}_mean")
            header.append(f"{score}_full")
        model_stats_df = pd.DataFrame(columns=header)

    # add the stats row to the stats file
    row = [time.time(), num_iteration, len(data)]
    for score in scores:
        mean = np.mean(cv_results[f"test_{score}"])
        full = ";".join(str(v) for v in cv_results[f"test_{score}"])
        row.append(mean)
        row.append(full)

    model_stats_df = model_stats_df.append(pd.Series(row, index=model_stats_df.columns), ignore_index=True)
    model_stats_df.to_csv(model_stats_path, index=False)

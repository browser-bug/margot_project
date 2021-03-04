import os
import time
import pandas as pd
import numpy as np
from pathlib import Path

def store_stats(metric_name, num_iteration, cv_results, data, target):
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

    plugin_working_dir = Path.cwd()
    app_name = split_all(plugin_working_dir)[-4]
    stats_directory = Path("/home/bernardo/Development/margot/benchmark/stats/" + app_name)
    # create the subdirectory for the application
    if not stats_directory.exists():
        Path.mkdir(stats_directory)

    # store the data and target tables for the current iteration
    data_path = stats_directory / f"data_itr_{num_iteration}.csv"
    target_path = stats_directory / f"target_{metric_name}_itr_{num_iteration}.csv"
    data.to_csv(data_path, index=False)
    target.to_csv(target_path, index=False)

    # add the stats row to the stats file
    model_stats_path = stats_directory / f"model_{metric_name}_stats.csv"
    if model_stats_path.exists():
        model_stats_df = pd.read_csv(model_stats_path)
    else:
        model_stats_df = pd.DataFrame(columns=['timestamp','nr_iter','nr_configs','r2_mean','mae_mean','r2_full','mae_full'])

    nr_configs = len(data)
    r2_full = ";".join(str(v) for v in cv_results['test_r2'])
    r2_mean = np.mean(cv_results['test_r2'])
    mae_full = ";".join(str(v) for v in cv_results['test_neg_mean_absolute_error'])
    mae_mean = np.mean(cv_results['test_neg_mean_absolute_error'])

    model_stats_df = model_stats_df.append(pd.Series([time.time(), num_iteration, nr_configs, r2_mean, mae_mean, r2_full, mae_full], index=model_stats_df.columns), ignore_index=True)
    model_stats_df.to_csv(model_stats_path, index=False)

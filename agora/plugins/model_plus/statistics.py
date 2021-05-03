import os
import time
import pandas as pd
import numpy as np
from pathlib import Path

def store_stats(metric_name, num_iteration, final_estimator_name, cv_train_results, holdout_results, score_metric_list, data, target):
    """
    metric_name : name of the EFP of interest
    num_iteration : number of iterations performed during agora learning process
    final_estimator_name : the name of the best estimator selected
    cv_train_results : dictionary containing for each key (estimator name) the cross validation (5 folds) results
    holdout_results : pandas dataframe containing for each row the estimator name and the score metrics computed on the holdout set
    score_metric_list : list of the score metrics evaluated during the modelling
    data : pandas dataframe representing the data set (software-knobs and input-features if any)
    target : pandas dataframe representing the target set (EFP of interest for each configuration in the data set)
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

    # compute the header of the file for each modelling technique
    for estimator_name, cv_result in cv_train_results.items():
        model_stats_path = stats_directory / f"model_{estimator_name}_{metric_name}_stats.csv"
        if model_stats_path.exists():
            model_stats_df = pd.read_csv(model_stats_path)
        else:
            header = ['timestamp','nr_iter','nr_configs', 'best_estimator_name', 'r2_final', 'mape_final', 'mae_final', 'mae_norm_final']
            for score in score_metric_list:
                header.append(f"{score}_cv_mean")
                header.append(f"{score}_cv_full")
            model_stats_df = pd.DataFrame(columns=header)

        # add the stats row to the stats file
        estimator_holdout_results = holdout_results[holdout_results['name'].str.contains(estimator_name)].reset_index(drop=True)
        row = [time.time(), num_iteration, len(data), final_estimator_name,
                estimator_holdout_results["r2"][0], estimator_holdout_results["mape"][0],
                estimator_holdout_results["mae"][0], estimator_holdout_results["mae_norm"][0]]
        for score in score_metric_list:
            mean = np.mean(cv_result[f"test_{score}"])
            full = ";".join(str(v) for v in cv_result[f"test_{score}"])
            row.append(mean)
            row.append(full)

        model_stats_df = model_stats_df.append(pd.Series(row, index=model_stats_df.columns), ignore_index=True)
        model_stats_df.to_csv(model_stats_path, index=False)


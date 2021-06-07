#
# Copyright (C) 2021 Bernardo Menicagli
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
# USA
#
"""Generates some useful statistics data.

This module contains functions to generate statistical data to be collected
during the learning process as CSV files.
"""

import os
import time
import pandas as pd
import numpy as np
from pathlib import Path

def store_stats(metric_name, num_iteration, final_estimator_name, cv_train_results, holdout_results, score_metric_list, data, target, stats_dir):
    """Store data files containing statistical infos collected during the learning process.

    Parameters
    ----------
    metric_name : `str`
        The EFP name.
    num_iteration : `int`
        The number of iterations performed.
    final_estimator_name : `str`
        The name of the best estimator found.
    cv_train_results : `dict` [`str`, `dict`]
        Array of scores for each run of the cross validation for each estimator tested.

        See https://scikit-learn.org/stable/modules/generated/sklearn.model_selection.cross_validate.html
        for details.
    holdout_results : `dict` [`str`, `dict`]
        For each model, a `dict` containing the final scores computed.
    score_metric_list : `list` [`str`]
        A list of the score metrics evaluated during the training phase.
    data : `pandas.DataFrame`
        A dataframe representing the matrix of predictors (i.e. software-knobs + input features).
    target : `pandas.DataFrame`
        A dataframe representing the matrix of target values (i.e. the metric of interest values).
    stats_dir : `str`
        Path of the directory where files will be stored.
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

    # compute the header of the file for each modelling technique
    for estimator_name, cv_result in cv_train_results.items():
        model_stats_path = stats_directory / f"model_{estimator_name}_{metric_name}_stats.csv"
        if model_stats_path.exists():
            model_stats_df = pd.read_csv(model_stats_path)
        else:
            header = ['timestamp', 'nr_iter', 'nr_configs',
                      'best_estimator_name', 'r2_final', 'mape_final']
            for score in score_metric_list:
                header.append(f"{score}_cv_mean")
                header.append(f"{score}_cv_full")
            model_stats_df = pd.DataFrame(columns=header)

        # add the stats row to the stats file
        estimator_holdout_results = holdout_results[holdout_results['name'].str.contains(estimator_name)].reset_index(drop=True)
        row = [time.time(), num_iteration, len(data), final_estimator_name,
                estimator_holdout_results["r2"][0], estimator_holdout_results["mape"][0]]
        for score in score_metric_list:
            mean = np.mean(cv_result[f"test_{score}"])
            full = ";".join(str(v) for v in cv_result[f"test_{score}"])
            row.append(mean)
            row.append(full)

        model_stats_df = model_stats_df.append(pd.Series(row, index=model_stats_df.columns), ignore_index=True)
        model_stats_df.to_csv(model_stats_path, index=False)


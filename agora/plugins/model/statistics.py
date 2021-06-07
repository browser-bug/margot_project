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

def store_stats(metric_name, num_iteration, cv_results, scores, data, target, stats_dir):
    """Store data files containing statistical infos collected during the learning process.

    Parameters
    ----------
    metric_name : `str`
        The EFP name.
    num_iteration : `int`
        The number of iterations performed.
    cv_results : `dict`
        Array of scores of the estimator for each run of the cross validation.

        See https://scikit-learn.org/stable/modules/generated/sklearn.model_selection.cross_validate.html
        for details.
    scores : `list` [`str`]
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

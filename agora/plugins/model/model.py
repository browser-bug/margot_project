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
"""Create a predicting model for a specific EFP.

This module contains functions that parse a list of parameters and create
a model using the method specified by the user. After applying a Cross-Validation
on it, extracts a list of metric scores and test them against their thresholds.
"""

import json
import numpy as np
from sklearn.base import RegressorMixin
from sklearn.linear_model import LinearRegression, Ridge
from sklearn.model_selection import cross_validate, KFold, LeavePOut
from sklearn.preprocessing import PolynomialFeatures
from sklearn.pipeline import Pipeline

def create_model(metric_name, num_iterations, model_params, data, target):
    """Create a predicting model for the specified metric of interest.

    Parameters
    ----------
    metric_name : `str`
        The EFP name.
    num_iterations : `int`
        The number of iterations performed.
    model_params : `dict` [`str`, `str`]
        The keys corresponds to the parameter name. The values are the parameter
        value and they need to be parsed accordingly (e.g. as an `int` or a python `dict`).
    data : `pandas.DataFrame`
        A dataframe representing the matrix of predictors (i.e. software-knobs + input features).
    target : `pandas.DataFrame`
        A dataframe representing the matrix of target values (i.e. the metric of interest values).

    Returns
    -------
    model : `tuple` [`bool`, `sklearn.base.RegressorMixin`]
        (`True`, model) if a model was successfully found, with the best model returned.

        (`False`, fake_model) if no model was found, returns a fake model.

    Notes
    -----
    This function can be extended by adding new methods inside the proper
    section. It should be pretty straight forward as the cross validation
    is equal regardless of the hyper_parameters or the method type.
    """
    print(model_params)
    algorithm = model_params['algorithm'] if 'algorithm' in model_params.keys() else 'linear'
    hyper_parameters = json.loads(model_params['hyper_parameters'].replace("'", "\"")) if 'hyper_parameters' in model_params.keys() else {}
    scoring_thresholds = json.loads(model_params['quality_threshold'].replace("'", "\"")) if 'quality_threshold' in model_params.keys() else {'r2':0.8, 'neg_mean_absolute_percentage_error':-0.1}

    # create the estimator based on the algorithm
    estimator = RegressorMixin()
    if algorithm == 'linear':
        print("Using an ordinary least squares linear regression estimator")
        estimator = Pipeline([('poly', PolynomialFeatures(degree=4)), ('linear', LinearRegression(normalize=True))])
    elif algorithm == 'ridge':
        print("Using a Ridge regression estimator")
        alpha = float(hyper_parameters['alpha']) if 'alpha' in hyper_parameters.keys() else 1.0
        solver = hyper_parameters['solver'] if 'solver' in hyper_parameters.keys() else 'auto'
        estimator = Pipeline([('poly', PolynomialFeatures(degree=4)), ('ridge', Ridge(alpha=alpha,normalize=True,solver=solver))])
    else:
        print("Unknown estimator name, returning empty object.")
        return False, estimator

    # produce the cross validation
    num_cv_folds = int(model_params['num_cv_folds']) if 'num_cv_folds' in model_params.keys() else 5
    num_samples = len(data)
    ratio_threshold = 0.6
    set_ratio = (num_samples - num_cv_folds) / num_samples
    if set_ratio < ratio_threshold:
        print(f"Not enough samples [{num_samples}] to perform a full CV. Using Leave2Out stategy instead...")
        p = 2
        cv_strategy = LeavePOut(p) if p < num_samples else LeavePOut(1)
    else:
        cv_strategy = KFold(n_splits=num_cv_folds)
    splits_iterator = cv_strategy.split(data, target)

    scores_list = list(scoring_thresholds.keys())
    cv_results = cross_validate(estimator, data, target, cv=splits_iterator,return_estimator=True, scoring=scores_list, verbose=0, n_jobs=4)

    # check if the score thresholds are verified
    best_estimator = [0] * len(cv_results['estimator'])
    is_model_good = True
    for scoring, threshold in scoring_thresholds.items():
        scores_max_pos = np.argmax(cv_results['test_' + scoring])
        best_estimator[scores_max_pos] += 1
        scores_mean = np.mean(cv_results['test_' + scoring])
        print("Comparing "+scoring, " MEAN[", scores_mean, "] with", threshold)
        if scores_mean < threshold:
            print("The scoring", scoring, "has not verified the threshold limit.")
            is_model_good = False

    # since the model is good we can fit it
    return is_model_good, cv_results['estimator'][np.argmax(best_estimator)]

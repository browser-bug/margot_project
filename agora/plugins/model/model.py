import json
import numpy as np
from sklearn.base import RegressorMixin
from sklearn.linear_model import LinearRegression, Ridge
from sklearn.model_selection import cross_validate, KFold, LeavePOut
from sklearn.preprocessing import PolynomialFeatures
from sklearn.pipeline import Pipeline

def create_model(metric_name, num_iterations, model_params, data, target):
    print(model_params)
    algorithm = model_params['algorithm'] if 'algorithm' in model_params.keys() else 'linear'
    hyper_parameters = json.loads(model_params['hyper_parameters'].replace("'", "\"")) if 'hyper_parameters' in model_params.keys() else {}
    scoring_thresholds = json.loads(model_params['quality_threshold'].replace("'", "\"")) if 'quality_threshold' in model_params.keys() else {'r2':0.7, 'explained_variance':0.7, 'neg_mean_absolute_error':-0.2}

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
    if num_samples <= num_cv_folds:
        p = 2
        cv_strategy = LeavePOut(p) if p < num_samples else LeavePOut(1)
    else:
        cv_strategy = KFold(n_splits=num_cv_folds)
    splits_iterator = cv_strategy.split(data, target)

    scores_list = list(scoring_thresholds.keys())
    cv_results = cross_validate(estimator, data, target, cv=splits_iterator,return_estimator=True, scoring=scores_list, verbose=1, n_jobs=-1)
    print(cv_results)

    # check if the score thresholds are verified
    best_estimator = [0] * num_cv_folds
    is_model_good = True
    for scoring, threshold in scoring_thresholds.items():
        scores_max_pos = np.argmax(cv_results['test_' + scoring])
        best_estimator[scores_max_pos] += 1
        scores_mean = np.mean(cv_results['test_' + scoring])
        print("Comparing "+scoring,cv_results['test_'+scoring]," MEAN[",scores_mean,"] with",threshold)
        if not scores_mean >= threshold:
            print("The scoring", scoring, "has not verified the threshold limit.")
            is_model_good = False

    # since the model is good we can fit it
    return is_model_good, cv_results['estimator'][np.argmax(best_estimator)]

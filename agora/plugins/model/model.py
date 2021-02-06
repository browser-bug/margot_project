import json
import numpy as np
from sklearn.base import RegressorMixin
from sklearn.linear_model import LinearRegression, Ridge
from sklearn.model_selection import cross_validate, cross_val_score, KFold, LeavePOut

# testing
from sklearn.datasets import make_regression

def create_model(model_params, data, target):
    print(model_params)
    algorithm = model_params['algorithm'] if 'algorithm' in model_params.keys() else 'linear'
    hyper_parameters = json.loads(model_params['hyper_parameters'].replace("'", "\"")) if 'hyper_parameters' in model_params.keys() else ''
    scoring_thresholds = json.loads(model_params['quality_threshold'].replace("'", "\"")) if 'quality_threshold' in model_params.keys() else {'r2':0.5, 'explained_variance':0.5, 'neg_mean_absolute_error':-0.2}

    # create the estimator based on the algorithm
    estimator = RegressorMixin()
    if algorithm == 'linear':
        print("Using an ordinary least squares linear regression estimator")
        estimator = LinearRegression()
    elif algorithm == 'ridge':
        print("Using a Ridge regression estimator")
        alpha = model_params['alpha'] if 'alpha' in model_params.keys() else 1.0
        solver = model_params['solver'] if 'solver' in model_params.keys() else 'auto'
        estimator = Ridge(alpha=alpha,solver=solver)
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

    #DEBUGGING
    # print(data)
    # print(target)
    # for train, test in cv_strategy.split(data, target):
        # # print("%s %s" % (train, test))
        # print(data.loc[train])
        # print(data.loc[test])
    ########

    cv_results = cross_validate(estimator, data, target, cv=splits_iterator, scoring=list(scoring_thresholds.keys()), verbose=1, n_jobs=-1)
    print(cv_results)

    # check if the score thresholds are verified
    for scoring, threshold in scoring_thresholds.items():
        scores_mean = np.mean(cv_results['test_' + scoring])
        print("Comparing "+scoring,cv_results['test_'+scoring]," MEAN[",scores_mean,"] with",threshold)
        if not scores_mean >= threshold:
            print("The scoring", scoring, "has not verified the threshold limit.")
            return False, estimator
    # since the model is good we can fit it
    return True, estimator

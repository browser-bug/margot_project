import json
import numpy as np
from sklearn.base import RegressorMixin
from sklearn.linear_model import LinearRegression, Ridge
from sklearn.model_selection import cross_validate, cross_val_score

# testing
from sklearn.datasets import make_regression

def create_model(model_params, data, target):
    print(model_params)
    algorithm = model_params['algorithm'] if 'algorithm' in model_params.keys() else 'linear'
    hyper_parameters = json.loads(model_params['hyper_parameters'].replace("'", "\"")) if 'hyper_parameters' in model_params.keys() else ''
    scoring_thresholds = json.loads(model_params['quality_threshold'].replace("'", "\"")) if 'quality_threshold' in model_params.keys() else {'r2':0.5, 'explained_variance':0.5, 'neg_mean_absolute_error':-0.2}
    cv_strategy = int(model_params['num_cv_folds']) if 'num_cv_folds' in model_params.keys() else 5
    # scores = model_params['scores'].split(';') if 'scores' in model_params.keys() else ['r2', 'explained_variance', 'neg_mean_squared_error']

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

    # data,target = make_regression(n_samples = 500,n_features=4, n_informative=4,n_targets=1)
    # print(data)
    # print(target)
    # produce the cross validation
    cv_results = cross_validate(estimator, data, target, cv=cv_strategy, scoring=list(scoring_thresholds.keys()), verbose=1, n_jobs=-1)
    print(cv_results)

    # check if the score thresholds are verified
    for scoring, threshold in scoring_thresholds.items():
        print("Comparing test_"+scoring,cv_results['test_'+scoring],"with",threshold)
        if not np.all(cv_results['test_' + scoring] >= threshold):
            print("The scoring", scoring, "has not verified the threshold limit.")
            return False, estimator
    # since the model is good we can fit it
    return True, estimator

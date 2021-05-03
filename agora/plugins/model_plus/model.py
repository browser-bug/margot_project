import json
import numpy as np
import pandas as pd
from sklearn.base import RegressorMixin
from sklearn.linear_model import LinearRegression, Ridge, ElasticNet
from sklearn.svm import SVR
from sklearn.neighbors import KNeighborsRegressor
from sklearn.model_selection import cross_validate, KFold, LeavePOut, train_test_split
from sklearn.metrics import r2_score, mean_absolute_error, mean_absolute_percentage_error
from sklearn.preprocessing import PowerTransformer
from sklearn.tree import DecisionTreeRegressor
from sklearn.pipeline import Pipeline

def cv_splits(num_cv_folds, data_train, target_train):
    # produce the cross validation
    num_samples = len(data_train)
    ratio_threshold = 0.75
    set_ratio = (num_samples - num_cv_folds) / num_samples
    if set_ratio < ratio_threshold:
        print(f"Not enough samples [{num_samples}] to perform a full CV. Using Leave2Out stategy instead...")
        p = 2
        cv_strategy = LeavePOut(p) if p < num_samples else LeavePOut(1)
    else:
        cv_strategy = KFold(n_splits=num_cv_folds)
    return cv_strategy.split(data_train, target_train)

def create_model(metric_name, num_iterations, model_params, data, target, is_last_iteration):
    print(model_params)
    num_cv_folds = int(model_params['num_cv_folds']) if 'num_cv_folds' in model_params.keys() else 5
    scoring_thresholds = json.loads(model_params['quality_threshold'].replace("'", "\"")) if 'quality_threshold' in model_params.keys() else {'r2':0.7, 'neg_mean_absolute_percentage_error':-0.1}

    # split the train and test sets
    data_train, data_test, target_train, target_test = train_test_split(data, target, test_size=0.25)

    # create a list of estimators based on the available models using default parameters
    estimators = {
            "linear_regressor": Pipeline([('linear', LinearRegression())]),
            "linear_regressor_boxcox": Pipeline([('transform',PowerTransformer("box-cox")),('linear', LinearRegression())]),
            "ridge_regressor": Pipeline([('transform',PowerTransformer("box-cox")),('ridge', Ridge(alpha=1.0,solver='auto'))]),
            "elastic_net": Pipeline([('transform',PowerTransformer("box-cox")),('elastic_net', ElasticNet(alpha=1.0,l1_ratio=1))]),
            "support_vector_regressor": Pipeline([('svr', SVR(kernel='rbf', C=500))]),
            "k_neighbors_regressor": Pipeline([('knr', KNeighborsRegressor(n_neighbors=min(5, abs(len(data_train)-2)),weights='distance'))]),
            "decision_tree_regressor": Pipeline([('decision_tree', DecisionTreeRegressor(criterion="mse"))])
            }

    scores_list = list(scoring_thresholds.keys())
    good_estimators = {} # <estimator_name, estimator>
    for name, estimator in estimators.items():
        print(f"Performing cross validation on regressor >> {name}")
        splits_iterator = cv_splits(num_cv_folds, data_train, target_train)
        cv_results = cross_validate(estimator, data_train, target_train, cv=splits_iterator, return_estimator=False, scoring=scores_list, verbose=0, n_jobs=4)

        is_model_good = True
        for scoring, threshold in scoring_thresholds.items():
            scores_mean = np.mean(cv_results['test_' + scoring])
            if scoring in "neg_mean_absolute_error":
                scores_mean = scores_mean / abs(np.max(target_train) - np.min(target_train))
            print("Comparing "+scoring,cv_results['test_'+scoring]," MEAN[",scores_mean,"] with",threshold)
            if not scores_mean >= threshold:
                print("The scoring", scoring, "has not verified the threshold limit.")
                is_model_good = False
        if is_model_good or is_last_iteration:
            good_estimators[name] = estimator

    estimators_scores = pd.DataFrame(columns=["name", "r2", "mape", "mae_norm"])
    for name, estimator in good_estimators.items():
        estimator.fit(data_train, target_train)
        test_prediction = estimator.predict(data_test)
        r2 = r2_score(target_test, test_prediction)
        mape = mean_absolute_percentage_error(target_test, test_prediction)
        mae = mean_absolute_error(target_test, test_prediction)
        mae_norm = mae / abs(np.max(target) - np.min(target))
        estimators_scores = estimators_scores.append({'name':name, 'r2':r2, 'mape':mape, 'mae_norm':mae_norm}, ignore_index=True)

    if not estimators_scores.empty:
        if is_last_iteration:
            final_estimator_name = str(estimators_scores.nsmallest(1, "mape").reset_index(drop=True)["name"][0])
        else:
            final_estimator_name = str(estimators_scores[estimators_scores["r2"] >= scoring_thresholds["r2"]].nsmallest(1, "mape").reset_index(drop=True)["name"][0])
        final_estimator = estimators[final_estimator_name].fit(data,target)
        print("Best estimator found!", final_estimator)
        return True, final_estimator
    else:
        return False, RegressorMixin()

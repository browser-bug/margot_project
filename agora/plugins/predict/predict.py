import pandas as pd

def create_predictions(samples_df, models):
    predictions_df = pd.DataFrame()

    for metric_name, model in models.items():
        print("Computing predictions for the metric:", metric_name)
        metric_predictions = model.predict(samples_df)
        predictions_df[metric_name + '_avg'] = metric_predictions
        # TODO: this will be set in case the model provides us a std for the prediction
        predictions_df[metric_name + '_std'] = 0

    return predictions_df

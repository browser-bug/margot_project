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
"""Generate the final predictions using the best models found.
"""

import pandas as pd

def create_predictions(samples_df, models):
    """Create a set of predictions for each EFPs.

    Parameters
    ----------
    samples_df : `pandas.DataFrame`
        The dataframe containing the list of predictors (i.e. a combination of
        software-knobs and input-features).
    models : `dict` [`str`, `sklearn.base.RegressorMixin`]
        The keys corresponds to the EFP name. The values are the best model found
        for that specific metric.

    Returns
    -------
    predictions : `pandas.DataFrame`
        A dataframe containing a list of predictions. For each EFP there are
        two columns corresponding to the **average** and the **standard_deviation**.

        | m1_avg | m1_std | m2_avg | m2_std | ... | mn_avg | mn_std |
    """
    predictions_df = pd.DataFrame()

    for metric_name, model in models.items():
        print("Computing predictions for the metric:", metric_name)
        metric_predictions = model.predict(samples_df)
        predictions_df[metric_name + '_avg'] = metric_predictions
        # TODO: this will be set in case the model provides us a std for the prediction
        predictions_df[metric_name + '_std'] = 0

    return predictions_df

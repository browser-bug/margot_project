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
"""Find cluster inside the input features space and generates their representatives.

This module contains functions to generate a list of centroids for each cluster
found inside the input features space. The centroids are used afterwards during
the final prediction phase.
"""

import pandas as pd
import numpy as np
from sklearn.preprocessing import StandardScaler
from sklearn.cluster import KMeans, DBSCAN

def create_cluster(cluster_params, features_data):
    """Create a set of experiments based on the specified design method.

    Parameters
    ----------
    cluster_params : `dict` [`str`, `str`]
        The keys corresponds to the parameter name. The values are the parameter
        value and they need to be parsed accordingly (e.g. as an `int` or a python `dict`).
    features_data : `pandas.DataFrame`
        The dataframe corresponding to the matrix of input features collected during DSE.

        | feat_1 | feat_2 | ... | feat_n |

    Returns
    -------
    centroids : `pandas.DataFrame`
        A dataframe containing a list of centroids for each cluster found.

        | cetroid_1 | cetroid_2 | ... | cetroid_n |

    Notes
    -----
    This function can be extended by adding new algorithms inside the proper section.
    A preprocessing phase is performed based on the parameters specified by the user.
    """
    do_preprocessing = int(cluster_params['preprocessing']) if 'preprocessing' in cluster_params.keys() else True
    algorithm = cluster_params['algorithm'] if 'algorithm' in cluster_params.keys() else 'kmeans'

    # Preprocessing phase
    scal = StandardScaler()
    if do_preprocessing:
        print("Scaling features matrix")
        features_data = scal.fit_transform(features_data)

    result = np.empty([0,features_data.shape[1]])

    print("Creating clusters using algorithm:", algorithm)

    if algorithm == "kmeans":
        method = cluster_params['init_method'] if 'init_method' in cluster_params.keys() else 'k-means++'
        n_clusters = int(cluster_params['num_clusters']) if 'num_clusters' in cluster_params.keys() else 5
        max_iter = int(cluster_params['max_num_iterations']) if 'max_num_iterations' in cluster_params.keys() else 300

        kmeans = KMeans(n_clusters=n_clusters, init=method, max_iter=max_iter, verbose=0 )
        kmeans_result = kmeans.fit(features_data)

        result = kmeans_result.cluster_centers_

    elif algorithm == "dbscan":
        method = cluster_params['init_method'] if 'init_method' in cluster_params.keys() else 'auto'
        eps = float(cluster_params['eps']) if 'eps' in cluster_params.keys() else 0.5
        neighborhood_size = float(cluster_params['neighborhood_size']) if 'neighborhood_size' in cluster_params.keys() else 5

        dbscan = DBSCAN(eps=eps, min_samples=neighborhood_size, algorithm=method, n_jobs=-1)
        dbscan_result = dbscan.fit(features_data)

        labels = dbscan_result.labels_
        n_clusters = len(set(labels)) - (1 if -1 in labels else 0)
        clusters = [features_data[labels == i] for i in range(n_clusters)]

        # TODO: change this to np.ndarray?
        result = []
        for cluster in clusters:
            result.append(np.mean(cluster, axis=0))
    else:
        print("Uknown doe algorithm, returning an empty doe")

    # Inverse transformation in case of scaled values
    if do_preprocessing:
        df = pd.DataFrame(data=scal.inverse_transform(result))
    else:
        df = pd.DataFrame(data=result)

    return df

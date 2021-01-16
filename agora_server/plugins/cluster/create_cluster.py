import pandas as pd
import numpy as np
from sklearn.preprocessing import StandardScaler
from sklearn.cluster import KMeans, DBSCAN

def create_cluster(cluster_params, features_matrix):
    print("Original features matrix")
    print(features_matrix)

    # Preprocessing phase
    scal = StandardScaler()
    do_preprocessing = int(cluster_params['preprocessing']) if 'preprocessing' in cluster_params.keys() else True
    if do_preprocessing:
        print("Scaling features matrix")
        features_matrix = scal.fit_transform(features_matrix)
        print(features_matrix)

    # algorithm = cluster_params['algorithm']
    algorithm = 'dbscan'
    result = np.empty([0,features_matrix.shape[1]])

    print("Creating clusters using algorithm:", algorithm)

    if algorithm == "kmeans":
        method = cluster_params['init_method'] if 'init_method' in cluster_params.keys() else 'k-means++'
        n_clusters = int(cluster_params['number_centroids']) if 'number_centroids' in cluster_params.keys() else 8
        max_iter = int(cluster_params['max_num_iterations']) if 'max_num_iterations' in cluster_params.keys() else 300

        kmeans = KMeans(n_clusters=n_clusters, init=method, max_iter=max_iter, verbose=1 )
        kmeans_result = kmeans.fit(features_matrix)

        result = kmeans_result.cluster_centers_

    elif algorithm == "dbscan":
        method = cluster_params['init_method'] if 'init_method' in cluster_params.keys() else 'auto'
        eps = float(cluster_params['eps']) if 'eps' in cluster_params.keys() else 0.5
        neighborhood_size = float(cluster_params['neighborhood_size']) if 'neighborhood_size' in cluster_params.keys() else 0.5

        dbscan = DBSCAN(eps=eps, min_samples=neighborhood_size, algorithm=method, n_jobs=-1)
        dbscan_result = dbscan.fit(features_matrix)

        labels = dbscan_result.labels_
        n_clusters = len(set(labels)) - (1 if -1 in labels else 0)
        clusters = [features_matrix[labels == i] for i in range(n_clusters)]

        # TODO: change this to np.ndarray?
        result = []
        for cluster in clusters:
            result.append(np.mean(cluster, axis=0))

        # result = dbscan_result
    else:
        print("Uknown doe algorithm, returning an empty doe")

    # Inverse transformation in case of scaled values
    if do_preprocessing:
        df = pd.DataFrame(data=scal.inverse_transform(result))
    else:
        df = pd.DataFrame(data=result)

    return df

# Plugin crs

This plugin leverage categorical regression splines to generate the response surface of the target metric. In particular, the implementation computes a spearman correlation matrix between the target metric and the first-order predictors with interaction to filter out useless predictors. Then it uses the R crs package to build the model that predicts the given metric.

## Requirements

The main script that perform the computation is written in R. However, since there is no a mature binding with Apache Cassandra, we use python script to interact with the storage. Therefore, we have the following requirements:
- Rscript (tested with version 3.43)
- python  (tested with version 2.7.14)

## Dependency

In the current implementation requires the following packages
- the crs and Hmisc package for R
```R
install.packages('Hmisc')
install.packages('crs', dependencies = TRUE)
```
- the cassandra driver for python
```sh
pip install --user --upgrade cassandra-driver
```
NOTE: the crs package requires OpenGL developer files, on fedora is enough to install the packages "mesa-libGL-devel mesa-libGLU-devel" from the repository.

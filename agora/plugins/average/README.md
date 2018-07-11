# Plugin average

This plugin perform computes the average and standard deviation of all the observed configurations.
It required a full-factorial design space exploration on the software-knobs and on the input features as well.
This plugin is very simple and it doesn't interpolate or learn the relations, it just computes the points

NOTE:
If there are no configuration for a specific entry in the model, it fails to build the model

## Requirements
- python3  (tested with version 3.6.5)
- the cassandra driver for python
```sh
pip install --user --upgrade cassandra-driver
```

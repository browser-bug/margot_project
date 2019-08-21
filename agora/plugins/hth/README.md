# Plugin hth

## Description

## Requirements

The main script that perform the computation is written in R.
Communication with Cassandra is done using JDBC driver for Cassandra.
- Rscript (tested with version 3.43)

## Dependency

In the current implementation requires the following packages
- the RJDBC, DiceDesign, DiceEval and DiceKriging package for R
```R
install.packages('RJDBC')
install.packages('DiceDesign')
install.packages('DiceEval')
install.packages('DiceKriging')
install.packages('tidyverse')
install.packages('mda')
install.packages('polspline')
install.packages('quadprog')
```

NOTE: the dependencies of tidyverse are the development files of curl and xml2.

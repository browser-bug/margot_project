import pandas as pd
import pyDOE2 as doe

from discretize import discretize

def create_doe(agora_properties, doe_params, levels, factors):
    df = pd.DataFrame()

    num_config_per_iter = int(agora_properties['number_configurations_per_iteration'])
    max_num_iter = int(agora_properties['max_number_iteration'])
    algorithm = doe_params['algorithm'] if 'algorithm' in doe_params.keys() else 'full-factorial'

    print("Creating DOE using algorithm: ", algorithm)

    if algorithm == "full-factorial":
        full_fact = doe.fullfact(levels)

        # generate new dataframe for human readability
        df = pd.DataFrame(data=full_fact, dtype=object)
        for i in df.index:
            for j in range(len(list(df.iloc[i]))):
                elem = factors[j][int(float(df.iloc[i][j]))]
                df.iloc[i][j] = elem

    elif algorithm == "lhs":
        num_factors = len(factors)
        num_samples = num_config_per_iter * max_num_iter
        criterion = doe_params['criterion'] if 'criterion' in doe_params.keys() else None

        lhs = doe.lhs(num_factors, samples=num_samples, criterion=criterion)
        print(lhs)

        df = pd.DataFrame(data=lhs, dtype=object)
        discretize(df, factors)
        df = df.drop_duplicates()
    else:
        print("Uknown doe algorithm, returning an empty doe")

    return df

import pandas as pd
import pyDOE2 as pydoe

def create_doe(agora_properties, doe_params, k_values):
    df = pd.DataFrame()

    num_config_per_iter = int(agora_properties['number_configurations_per_iteration'])
    max_num_iter = int(agora_properties['max_number_iteration'])
    algorithm = doe_params['algorithm'] if 'algorithm' in doe_params.keys() else 'full-factorial'

    print("Creating DOE using algorithm: ", algorithm)

    if algorithm == "full-factorial":
        levels = [len(k_values[k_name]) for k_name in k_values.keys()]
        full_fact = pydoe.fullfact(levels)
        df = pd.DataFrame(data=full_fact, dtype=object, columns=k_values.keys())

        # map every value to its corresponding factor
        for k_name, values in df.iteritems():
            df[k_name] = values.apply(lambda x: k_values[k_name][int(float(x))])

    elif algorithm == "lhs":
        num_factors = len(k_values.keys())
        num_samples = num_config_per_iter * max_num_iter
        criterion = doe_params['criterion'] if 'criterion' in doe_params.keys() else None

        lhs = pydoe.lhs(num_factors, samples=num_samples, criterion=criterion)
        df = pd.DataFrame(data=lhs, dtype=object, columns=k_values.keys())

        # map every value to its corresponding factor rounding it for discretization
        max_kb_idx = dict((k, (lambda x: len(x)-1)(v)) for k, v in k_values.items())
        for k_name, values in df.iteritems():
            df[k_name] = values.apply(lambda x: k_values[k_name][round(float(x) * max_kb_idx[k_name])])
        df = df.drop_duplicates()

    else:
        print("Uknown doe algorithm, returning an empty doe")

    print(df)
    return df

def create_total_configuration_table(k_values):
    levels = [len(k_values[k_name]) for k_name in k_values.keys()]
    full_fact = pydoe.fullfact(levels)
    df = pd.DataFrame(data=full_fact, dtype=object, columns=k_values.keys())

    # map every value to its corresponding factor
    for k_name, values in df.iteritems():
        df[k_name] = values.apply(lambda x: k_values[k_name][int(float(x))])

    return df

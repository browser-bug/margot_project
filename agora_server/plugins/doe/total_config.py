import pandas as pd
from pyDOE2 import fullfact

def create_total_configuration_table(levels, factors):
    full_fact = fullfact(levels)

    # generate new dataframe for human readability
    df = pd.DataFrame(data=full_fact, dtype=object)
    for i in df.index:
        for j in range(len(list(df.iloc[i]))):
            elem = factors[j][int(float(df.iloc[i][j]))]
            df.iloc[i][j] = elem
    return df

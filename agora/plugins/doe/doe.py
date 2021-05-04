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
"""Create a collection of experiments for DSE.

This module contains functions to generate a list of software-knobs configurations
to be exploited during DSE and a table containing all the possible combinations
(total configuration table).
"""

import pandas as pd
import pyDOE2 as pydoe

def create_doe(agora_properties, doe_params, k_values):
    """Create a set of experiments based on the specified design method.

    Parameters
    ----------
    agora_properties : `dict` [`str`, `str`]
        The keys corresponds to the agora property name. The values are the parameter
        value and they need to be parsed accordingly.
    doe_params : `dict` [`str`, `str`]
        The keys corresponds to the parameter name. The values are the parameter
        value and they need to be parsed accordingly (e.g. as an `int` or a python `dict`).
    k_values : `dict` [`str`, `list`]
        The keys corresponds to the software-knob name. The values are a list
        of the corresponding domain.

    Returns
    -------
    doe : `pandas.DataFrame`
        A dataframe containing a list of software-knobs configurations.

        | kb_1 | kb_2 | ... | kb_n |

    Notes
    -----
    This function can be extended by adding new algorithms inside the proper section.
    """
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

    return df

def create_total_configuration_table(k_values):
    """Create a table containing all the possible configurations available.

    Parameters
    ----------
    k_values : `dict` [`str`, `list`]
        The keys corresponds to the software-knob name. The values are a list
        of the corresponding domain.

    Returns
    -------
    total_configurations : `pandas.DataFrame`
        A dataframe containing a list of all the possible software-knobs configurations.

        | kb_1 | kb_2 | ... | kb_n |
    """
    levels = [len(k_values[k_name]) for k_name in k_values.keys()]
    full_fact = pydoe.fullfact(levels)
    df = pd.DataFrame(data=full_fact, dtype=object, columns=k_values.keys())

    # map every value to its corresponding factor
    for k_name, values in df.iteritems():
        df[k_name] = values.apply(lambda x: k_values[k_name][int(float(x))])

    return df

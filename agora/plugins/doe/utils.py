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
"""Utility functions for the DOE generation process.
"""

from sklearn.preprocessing import LabelEncoder

dtype_conv = {'string': str, 'short int': int, 'int8_t': int, 'int16_t': int, 'int32_t': int, 'int64_t': int, 'uint8_t': int, 'uint16_t': int, 'uint32_t': int, 'uint64_t': int, 'char': str, 'int': int, 'long int': int, 'long long int': int, 'int_fast8_t': int, 'int_fast16_t': int, 'int_fast32_t': int, 'int_fast64_t': int, 'int_least8_t': int, 'int_least_16_t': int, 'int_least32_t': int, 'int_least64_t': int, 'unsigned short int': int, 'unsigned char': str, 'unsigned int': int, 'unsigned long int': int, 'unsigned long long int': int, 'uint_fast8_t': int, 'uint_fast16_t': int, 'uint_fast32_t': int, 'uint_fast64_t': int, 'uint_least8_t': int, 'uint_least16_t': int, 'uint_least32_t': int, 'uint_least64_t': int, 'intmax_t': int, 'intptr_t': int, 'uintmax_t': int, 'uintptr_t': int, 'float': float, 'double': float, 'long double': float}
def convert_types(df, knob_types):
    """Converts each column of the input dataframe to the corresponding knob datatype.

    Parameters
    ----------
    df : `pandas.DataFrame`
        The dataframe containing a list of software-knobs configurations.
    knob_types : `dict` [`str`, `str`]
        The keys are the software-knob name, the values are their type.
    """
    for k_name, k_type in knob_types.items():
        print(f'Converting {k_name} from type {df[k_name].dtype} to type {dtype_conv[k_type]}')
        df[k_name] = df[k_name].astype(dtype_conv[k_type])

def encode_data(df, knob_types):
    """For each string type software-knob, creates a `LabelEncoder`.

    Parameters
    ----------
    df : `pandas.DataFrame`
        The dataframe containing a list of software-knobs configurations.
    knob_types : `dict` [`str`, `str`]
        The keys are the software-knob name, the values are their type.

    Returns
    -------
    encoders : `dict` [`str`, `LabelEncoder`]
        The keys are the software-knob name, the values are a `LabelEncoder`.

    Notes
    -----
    The encoders are needed for the modelling phase since we cannot manage
    string type values inside a regressor.
    """

    encoders = {}

    for knob_name, knob_type in knob_types.items():
        if knob_type == 'string':
            enc = LabelEncoder()
            enc.fit(df[knob_name])
            encoders[knob_name] = enc

    return encoders

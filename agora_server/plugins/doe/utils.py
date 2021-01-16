import pandas as pd
from sklearn.preprocessing import LabelEncoder

dtype_conv = {'string': str, 'short int': int, 'int8_t': int, 'int16_t': int, 'int32_t': int, 'int64_t': int, 'uint8_t': int, 'uint16_t': int, 'uint32_t': int, 'uint64_t': int, 'char': str, 'int': int, 'long int': int, 'long long int': int, 'int_fast8_t': int, 'int_fast16_t': int, 'int_fast32_t': int, 'int_fast64_t': int, 'int_least8_t': int, 'int_least_16_t': int, 'int_least32_t': int, 'int_least64_t': int, 'unsigned short int': int, 'unsigned char': str, 'unsigned int': int, 'unsigned long int': int, 'unsigned long long int': int, 'uint_fast8_t': int, 'uint_fast16_t': int, 'uint_fast32_t': int, 'uint_fast64_t': int, 'uint_least8_t': int, 'uint_least16_t': int, 'uint_least32_t': int, 'uint_least64_t': int, 'intmax_t': int, 'intptr_t': int, 'uintmax_t': int, 'uintptr_t': int, 'float': float, 'double': float, 'long double': float}
def convert_types(df, knob_types):
    for k_name, k_type in knob_types.items():
        print(f'Converting {k_name} from type {df[k_name].dtype} to type {dtype_conv[k_type]}')
        df[k_name] = df[k_name].astype(dtype_conv[k_type])

def encode_data(df, knob_types):
    """ Returns a dictionary of shape:
        key:    knob_name
        value:  encoder
    """

    encoders = {}

    for knob_name, knob_type in knob_types.items():
        if knob_type == 'string':
            enc = LabelEncoder()
            enc.fit(df[knob_name])
            encoders[knob_name] = enc

    return encoders

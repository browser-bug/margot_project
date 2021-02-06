import pandas as pd

dtype_conv = {'string': str, 'short int': int, 'int8_t': int, 'int16_t': int, 'int32_t': int, 'int64_t': int, 'uint8_t': int, 'uint16_t': int, 'uint32_t': int, 'uint64_t': int, 'char': str, 'int': int, 'long int': int, 'long long int': int, 'int_fast8_t': int, 'int_fast16_t': int, 'int_fast32_t': int, 'int_fast64_t': int, 'int_least8_t': int, 'int_least_16_t': int, 'int_least32_t': int, 'int_least64_t': int, 'unsigned short int': int, 'unsigned char': str, 'unsigned int': int, 'unsigned long int': int, 'unsigned long long int': int, 'uint_fast8_t': int, 'uint_fast16_t': int, 'uint_fast32_t': int, 'uint_fast64_t': int, 'uint_least8_t': int, 'uint_least16_t': int, 'uint_least32_t': int, 'uint_least64_t': int, 'intmax_t': int, 'intptr_t': int, 'uintmax_t': int, 'uintptr_t': int, 'float': float, 'double': float, 'long double': float}
def convert_types(df, features_types):
    for f_name, f_type in features_types.items():
        print(f'Converting {f_name} from type {df[f_name].dtype} to type {dtype_conv[f_type]}')
        df[f_name] = df[f_name].astype(dtype_conv[f_type])

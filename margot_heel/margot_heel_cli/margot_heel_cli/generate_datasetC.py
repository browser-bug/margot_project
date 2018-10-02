import os                                # for creating the path
import errno                             # for checking before creating a path


def generate_block_body( block_model, cc ):
  """
  Generates the per block code
  """

  cc.write('\n\n// The data structure of the dataset for the block of code {0} - C version\n'.format(block_model.block_name))
  
  cc.write('\n\ntypedef struct datasetC_{0}\n'.format(block_model.block_name))
  cc.write('{\n')
  for input_data_model in block_model.datasets_model[0].input_data_models:
      if (input_data_model.type == "string"):
          cc.write('\tchar* {0};\n'.format(input_data_model.name))
      else:
          cc.write('\t{0} {1};\n'.format(input_data_model.type, input_data_model.name))
  cc.write('} ')
  cc.write('datasetC_{0};\n'.format(block_model.block_name))




def generate_datasetC_h( block_models, output_folder ):
  """
  This function generates the actual datasetC header file
  """


  # open the output file
  with open(os.path.join(output_folder, 'datasetC.h'), 'w') as cc:

    cc.write('#ifndef DATASETC\n')
    cc.write('#define DATASETC\n')

    # put some new lines
    cc.write('\n\n')

    # write the disclaimer
    cc.write('\n\n\n/**\n')
    cc.write(' * WARNING:\n')
    cc.write(' * This file is autogenerated from the "margotcli" utility function.\n')
    cc.write(' * Any changes to this file might be overwritten, thus in order to \n')
    cc.write(' * perform a permanent change, please update the configuration file \n')
    cc.write(' * and re-generate this file. \n')
    cc.write(' */ \n\n\n')

    # generate the code block-specific code
    for block_name in block_models:
        generate_block_body( block_models[block_name], cc )

    # write some trailer spaces
    cc.write('\n\n')
    cc.write('#endif /* DATASETC */\n\n')
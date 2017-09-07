from __future__ import print_function
import os                                # for creating the path
import errno                             # for checking before creating a path

from .generate_margot_hpp import generate_margot_hpp
from .generate_margot_cc import generate_margot_cc
from .generate_margot_op_struct_hpp import generate_margot_structure_hpp
from .generate_op_list import generate_op_lists
from .generate_margot_h import generate_margot_h
from .generate_margot_c import generate_margot_c
from .generate_margot_logger_hpp import generate_margot_logger_hpp


def mkdir_p(p):
  """
  Helper function that emulate the command "mkdir -p"
    p -> the path to be created
  """
  try:
    os.makedirs(p)
  except OSError as exc: # Python >2.5
    if exc.errno == errno.EEXIST and os.path.isdir(p):
      pass
    else:
      print('[SYS ERROR]: Unable to create the path {0}'.format(p))
      sys.exit(-1)




def generate_source_files( block_models, op_lists, output_folder ):
  """
  This is the main function that generates the required output files for the
  high-level interface towards mMARGOTt.
  It assumes that the model is correct, no additional checks are performed
  """


  # create the output folder if needed
  mkdir_p(output_folder)

  # generate the op list source files
  generate_op_lists(op_lists, output_folder)

  # generate the margot_op_struct.h source file
  generate_margot_structure_hpp( block_models, op_lists, output_folder)

  # generate the application specific logger source file
  generate_margot_logger_hpp(output_folder)

  # generate the margot.hpp source file
  generate_margot_hpp(block_models, op_lists, output_folder)

  # generate the margot.h source file
  generate_margot_h(block_models, op_lists, output_folder)

  # generate the margot_interface_c.cc file
  generate_margot_c(block_models, op_lists, output_folder)

  # generate the margot.cc source file
  generate_margot_cc(block_models, op_lists, output_folder)

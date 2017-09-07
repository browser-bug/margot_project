from __future__ import print_function                            # the new print statement
import xml.etree.ElementTree as ET                               # get the xml parser from python
from xml.parsers.expat import ParserCreate, ExpatError, errors   # get the error message from the message
import sys                                                       # to exit
import re                                                        # to get the namespace


def print_node_attributes( node ):
  """
  Helper function that print on the standard output
  the attribute of an xml node

  @param node The target xml node
  """

  # check if the attribute has no parameters
  if not node.attrib:
    print('                \t- The element doesn\'t have any attribute')
    return

  # otherwise pring the node attributes
  for a in node.attrib:
    print('                \t- "{0}"'.format(a))



def parse_xml_file( file_path ):
  """
  Parse the XML file at path "path" and return the following tuple:
    (xml_root, namespace)
  """
  try:
    with open(file_path, 'r') as xml_file:
      xml_tree = ET.parse(xml_file)
  except ET.ParseError as err:
    lineno, column = err.position
    print('[PARSER ERROR]: {0}!'.format(err))
    sys.exit(-1)
  except IOError:
    print('[I/O ERROR]: Unable to open or read the file "{0}"'.format(file_path))
    sys.exit(-1)

  # get the root element
  xml_root = xml_tree.getroot()

  # get the namespace
  namespace = re.match('\{.*\}', xml_root.tag)
  namespace = namespace.group(0) if namespace else ''

  # return the information
  return (xml_root, namespace)


def get_elements( xml_root, element_name, required = False, namespace = '', exclusive = False, unique = False ):
  """
  Retrieves all the elements "element_name" from the xml_root "xml_root".
  If required is false, then throws an exception if it doesn't find any element "element_name"
  Element_name might be a list.
  If exclusive is True, then if the root contains some other elements, a warning is written to the std output
  """

  # all the elemnts that must be retrieved
  my_target_elements = []


  # check if the required element is single
  if isinstance(element_name, str):
    my_target_elements.append(element_name)
  else:
    my_target_elements.extend(element_name)


  # get all the xml element that should be examinated
  xml_elements = []
  if exclusive:
    for xml_element in xml_root:
      # check if its tag belongs to the required ones
      if xml_element.tag not in my_target_elements:
        print('[WARNING] Found unexpected element with tag "{0}" in element with tag "{1}"'.format(xml_element.tag, xml_root.tag))
      else:
        xml_elements.append(xml_element)
  else:
    for element_name in my_target_elements:
      for xml_element in xml_root.findall('{0}{1}'.format(namespace, element_name)):
        xml_elements.append(xml_element)

  # check if we have found something
  if required:
    if not xml_elements:
      print('[LOGIC ERROR] An element with tag "{0}" must include also element(s) with the following tag(s):'.format(xml_root.tag))
      for element in my_target_elements:
        print('                - "{0}"'.format(element))
      print('              Error: unable to find any of those elements in the following xml element:')
      print('                     TAG NAME:   "{0}"'.format(xml_root.tag))
      print('                     ATTRIBUTES: [{0}]'.format('", "'.join(xml_root.attrib)))
      sys.exit(-1)

  # check if a cunter of elements are setted
  if unique:
    if len(xml_elements) > 1:
      print('[WARNING] An element with tag <{0}> has more children with tag(s) "{1}" then expected: we are using only the first one!'.format(xml_root.tag, ','.join(my_target_elements)))

  # otherwise i might be able to return the xml_elements
  return xml_elements


def get_parameter( xml_element, attrib_name, my_target_type=str, required=True, prefixed_values = [] ):
  """
  Retrieves the value of a parameter if any
  """

  try:
    # retrieve the parameter
    param_str = xml_element.attrib[attrib_name]

    # check if we need to convert it
    if my_target_type != str:
      param = my_target_type(param_str)
    else:
      param = param_str

    # check if it must belong to a prefixed set of values
    if prefixed_values:
      if (param not in prefixed_values) and (param.upper() not in prefixed_values):
        print('[LOGIC ERROR]: Wrong value for the attribute "{0}"!'.format(attrib_name))
        print('               The attribute "{0}" in an element with tag <{1}> should assume one of the following values: ["{2}"]'.format(attrib_name, xml_element.tag, '", "'.join(prefixed_values)))
        print('               Actual value of the attribute: "{0}"'.format(param))
        sys.exit(-1)

    # return the parameter
    return param

  except KeyError:
    if required:
      print('[LOGIC ERROR]: Unable to find attribute "{0}"!'.format(attrib_name))
      print('                A tag <{0}> must define the attribute also the attribute "{1}"'.format(xml_element.tag, attrib_name))
      print('                Actual attribute(s) of the element:')
      print_node_attributes(xml_element)
      sys.exit(-1)
    return None
  except ValueError:
    print('[LOGIC ERROR]: Unable to understand the attribute "{0}"!'.format(attrib_name))
    print('                The value of the attribute "{0}" of a element <{1}> must be {2}!'.format(attrib_name, xml_element.tag, my_target_type))
    print('                This is the actual value for the attribute: "{1}"'.format(xml_element.attrib[attrib_name]))
    sys.exit(-1)

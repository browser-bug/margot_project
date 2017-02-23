import sys
from .model_op import OperatingPointModel

class OperatingPointListModel:
	"""
	This class represents a list of Operating Points:
		- The list of OperatingPointModel
		- The dictionary used to translate the Operating Points string -> int
		- The reverse dictionary int -> string
		- The name of the block
	"""

	def __init__(self):
		self.ops = []
		self.translator = {}
		self.reverse_translator = {}
		self.name = 'elaboration'

	def get_op (self, knob_map):
		"""
		knob map must have the same format as in operatin point model:
		key == knob name
		value = knob value

		"""
		if len(self.ops) == 0:
			print ("attempt to get an OP from an empty oplist")
			sys.exit(-1)

		if len(self.ops[0].knobs) != len(knob_map):
			print ("attempt to get an OP with a wrong knob map")
			sys.exit(-1)
		op_to_return = OperatingPointModel()
		for op in self.ops:
			flag=True
			for knob in op.knobs:
				if float(op.knobs[knob]) != float(knob_map[knob]):
					flag=False
			if flag:
				if not op_to_return.knobs:
					op_to_return=op
				else:
					print ("oplist contains two points with the same knobs")
					sys.exit(-1)
		return op_to_return

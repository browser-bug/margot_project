class ConstraintModel:
	"""
	Represents a constraint in the optimization problem.
	In particular, it conrains the following information:
		- the reference to the goal
		- the priority level
		- the target field of the operating point
	"""

	def __init__(self):
		self.goal_ref = ""
		self.priority = ""
		self.target_metric = ""
		self.target_knob = ""

from setuptools import setup

setup(
	name='margot_heel_cli',
	version='1.0',
	description='The mARGOt command line interface',
	author='Davide Gadioli',
	author_email='davide.gadioli@polimi.it',
	license='LGPL v2.1',
	packages=['margot_heel_cli'],
	scripts=['bin/margot_cli'],
	install_requires=[
		'xml',
	],
	zip_safe=False
	)

# -*- coding: utf-8 -*-

import os
import platform
import re

#from distutils import dep_util
#from distutils import log
from distutils.core import setup
from distutils.core import Extension
#from distutils.cmd import Command
#from distutils.command.build_scripts import build_scripts as _build_scripts
#from distutils.command.install_scripts import install_scripts as _install_scripts



extcoderunner_src = ["python/ext/pycoderunner.c",
					"src/coderunner.c",]
extcoderunner_include = ['include', "python/ext/object_impl",]



setup(name='childprocess',
		version='1.0',
		description='Class for creating and waiting for child processes complete',
		packages=['coderunner',],
		package_dir={'': 'lib'},
		ext_modules=[
			Extension("coderunner", extcoderunner_src, include_dirs=extcoderunner_include)
		]
	)



# vim: ts=4 sw=4 ai nowrap

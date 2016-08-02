
# -*- coding: utf-8 -*-

import os
import platform
import re

from distutils.core import setup
from distutils.core import Extension



extcoderunner_src = ["python/ext/pycoderunner.c",
					"src/coderunner.c",]
extcoderunner_include = ['include', "python/ext/object_impl",]



setup(name='childprocess',
		version='1.1.0',
		description='Class for creating and waiting for child processes complete',
		ext_modules=[
			Extension("coderunner", extcoderunner_src, include_dirs=extcoderunner_include)
		]
	)



# vim: ts=4 sw=4 ai nowrap

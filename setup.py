#!/usr/bin/env python
"""
setup.py file for pylibmmal
"""
import glob
import platform
from setuptools import setup, Extension

NAME = 'pylibmmal'
VERSION = '0.0'
VER_FILE = 'src/pylibmmal.c'
lines = [x for x in open(VER_FILE).read().split("\n") if "#define" in x and "_VERSION_" in x and "\"" in x]


if len(lines) > 0:
    VERSION = lines[0].split("\"")[1]
else:
    raise Exception('Unable to find _VERSION_ in {}'.format(VER_FILE))


if not platform.machine().startswith('arm'):
    raise Exception('{} only support raspberry'.format(NAME))


if platform.system().lower() != 'linux':
    raise Exception('{} only support linux'.format(NAME))


pylibmmal_module = Extension(NAME,
                             sources=glob.glob('src/*.c'),
                             library_dirs=['/opt/vc/lib'],
                             libraries=['bcm_host', 'mmal', 'mmal_util', 'mmal_core'],
                             include_dirs=['/opt/vc/include', '/opt/vc/include/interface/mmal'])


setup(
    name=NAME,
    version=VERSION,
    license='MIT',
    author='Amaork',
    author_email='amaork@gmail.com',
    url='https://github.com/amaork/pylibmmal',
    description='Raspberry Multi-Media Abstraction Layer Library.',
    ext_modules=[pylibmmal_module],
    classifiers=[
        'Programming Language :: Python',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 3',
    ],
)



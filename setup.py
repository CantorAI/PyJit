import sys,os
from setuptools import setup,Extension
import glob
from pathlib import Path

cwd = Path(__file__).parent.absolute()
os.chdir(cwd)

ver = os.environ.get('RELEASE_VERSION')
if ver == None:
    ver = '0.1.0.0'
    print("ENV: RELEASE_VERSION is not set,set to default")
src_cpp = []
src_cpp += glob.glob('common/*.cpp')
src_cpp += glob.glob('grus/*.cpp')


pid = os.getpid()
libs =[]
macros =[('NPY_NO_DEPRECATED_API', 'NPY_1_7_API_VERSION')]
import platform
if platform.system() == 'Windows':
    macros += [('WIN32', '1')]
    libs +=['user32','Ole32']
else:
    libs +=['uuid']

inc_dirs = [
    './python/',
    './common/',
    './grus/'
    ]
class get_numpy_include(object):
    def __str__(self):
        import numpy
        return numpy.get_include()

inc_dirs+=[get_numpy_include()]

grus_module = Extension('pyjit.grus',
                    sources = src_cpp,
                    include_dirs=inc_dirs,
                    libraries =libs,
                    define_macros=macros)

long_description ='''
    PyJit to embed c++ into python
    and compile code just in time,
    for examples and APIs, visit [pyjit](http://pyjit.org)
    
'''
setup(
    name="pyjit",
    version=ver,
    author="PyJit Team",
    author_email="Olivia@pyjit.org",
    description="PyJit to embed c++ into python",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="http://www.pyjit.org",
    project_urls={
        "Bug Tracker": "http://www.pyjit.org/pyjitproject/issues",
    },
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
    install_requires=[
       "setuptools",
       "numpy"
    ],

    entry_points={
        "console_scripts": [
            "pyjit=pyjit:generate",
        ]
    },
    package_data={'pyjit':['./Jit_Object.h','./Jit_Host.h']},
    package_dir={"pyjit": "./python"},
    packages =["pyjit"],

    include_package_data=False,

    python_requires=">=3.8",
    ext_modules = [grus_module]
)

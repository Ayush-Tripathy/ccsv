from setuptools import setup, Extension
from Cython.Build import cythonize
from distutils.command.build_ext import build_ext as _build_ext
import os


class BuildExt(_build_ext):
    def run(self):
        super().run()

        # Copy the .pyi files to the build directory
        build_pyi = os.path.join(self.build_lib, "")
        os.makedirs(build_pyi, exist_ok=True)
        os.system(f"cp python/*.pyi {build_pyi}")

        # Delete python directory
        os.system(f"rm -r {os.path.join(self.build_lib, 'python')}")


extension = Extension(
    name="ccsv",
    sources=["python/ccsv_python.c", "python/putils.c", "src/ccsv.c"],
    include_dirs=["include"],
    extra_compile_args=["-O3"],
    libraries=[],
)

setup(
    name="ccsv",
    version="0.1",
    description="Read/Write CSV files",
    ext_modules=cythonize([extension]),
    packages=[""],
    package_data={"": ["python/*.pyi"]},
    cmdclass={"build_ext": BuildExt},
    # include_package_data=True,
)

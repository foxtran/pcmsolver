. "$CI_SCRIPTS/common.sh"

python setup.py --cxx=g++ --cc=gcc --fc=gfortran --type=release
cd build
make
ctest
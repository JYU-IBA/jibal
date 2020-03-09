# JIBAL

Library that provides stopping forces, straggling models, cross sections and various atomic data e.g. masses and abundances for typical IBA applications.

## Installation from sources

Install GNU Scientific library and GNU Autotools and a compiler. After this, on Linux and MacOS you should run:

~~~~
./autogen.sh
./configure
make
make install
~~~~

## Usage of bundled tools

The get\_stop program can be used to extract stopping like this:

~~~~
get_stop 4He SiO2 "1 MeV"
~~~~

or for example

~~~~
get_stop 4He "Si0.33 O0.33 N0.33" 1MeV 100keV 10MeV
~~~~

This should (interpolation and other numerical issues aside) reproduce SRIM 2013 stopping values with the data included in the distribution.

## Using the jibal library with your own programs

When compiling your programs against jibal you can get the compiler flags with pkg-config (assuming pkg-config finds jibal.pc file)

~~~~
pkg-config --cflags --libs jibal
~~~~

Otherwise just use
~~~~
gcc -I/usr/local/include -I/usr/local/include/jibal -L/usr/local/lib -ljibal -lm -lgsl -lgslcblas -lm ...
~~~~

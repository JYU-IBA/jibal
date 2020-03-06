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

## Usage

The get\_stop program can be used to extract stopping like this:

~~~~
get_stop 4He SiO2 "1 MeV"
~~~~

Please note that the current stopping data included in the library is not only electronic stopping, but the library assumes it is, so the stopping values are wrong 
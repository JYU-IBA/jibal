# JIBAL

Library that provides stopping forces, straggling models, cross sections and various atomic data e.g. masses and abundances for typical IBA applications.

## Installation from sources

~~~~
./autogen.sh
./configure
make
make install
~~~~

## Usage

The get\_stop function can be used to extract stopping like this:

~~~~
get_stop 4He SiO2 "1 MeV"
~~~~

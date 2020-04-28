# JIBAL

Library that provides stopping forces, straggling models, cross sections and various atomic data e.g. masses and abundances for typical IBA applications.

## Installation from sources

See [installation instructions](INSTALL.md)

## Datafiles

The library is pretty useless without data files. You can obtain some [elsewhere](http://users.jyu.fi/~jaakjuli/jibal/data/).

## Usage of bundled tools

### Get_stop

The get\_stop program can be used to extract stopping like this:
~~~~
get_stop 4He SiO2 "1 MeV"
~~~~

or for example (stopping in 100 keV steps)
~~~~
get_stop 4He "Si0.33 O0.33 N0.33" 1MeV 100keV 10MeV
~~~~
This should (interpolation and other numerical issues aside) reproduce SRIM 2013 stopping values with the data included in the distribution.


You can also do energy loss calculations in layers, e.g.
~~~~
get_stop 4He Au 2MeV 1000tfu
~~~~

Elements are assumed to have natural isotopic composition, unless you tell otherwise, e.g.
~~~~
get_stop 4He "7Li0.60 6Li0.40" 2MeV 1000tfu
~~~~
Note that this results in two elements, but this is fine. 

### Jibaltool

Run
    
    $ jibaltool
    
to get a short help. You can extract entire files of stopping data and see the current JIBAL configuration.

## Using the jibal library with your own programs

Using CMake is preferred, see directory "demo" for an example of a C++ program using Jibal.

Alternatively when compiling your programs against jibal you can get the compiler flags with pkg-config (assuming pkg-config finds jibal.pc file)

~~~~
pkg-config --cflags --libs jibal
~~~~


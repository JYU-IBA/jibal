# JIBAL

Library that provides stopping forces, straggling models, cross sections and various atomic data e.g. masses and abundances for typical IBA applications.

## Installation from sources

See [installation instructions](INSTALL.md)

## Datafiles

The library is pretty useless without data files. You can obtain some [elsewhere](http://users.jyu.fi/~jaakjuli/jibal/data/).

There is a tool called *jibal_bootstrap* you can use to make a user configuration. See instructions of the data package.

## Usage of bundled tools

### Get_stop

The get\_stop program can be used to extract stopping like this:
~~~~
get_stop 4He "1 MeV" -l SiO2
~~~~

or for example (stopping in 100 keV steps)
~~~~
get_stop 4He 1MeV 100keV 10MeV -l "Si0.33 O0.33 N0.33" 
~~~~
This should (interpolation and other numerical issues aside) reproduce SRIM 2013 stopping values with the data included in the distribution.


You can also do energy loss calculations in layers, e.g.
~~~~
get_stop 4He 2MeV -l Au -t 1000tfu
~~~~

Elements are assumed to have natural isotopic composition, unless you tell otherwise, e.g.
~~~~
get_stop 4He 2MeV -l "7Li0.60 6Li0.40" -t 1000tfu
~~~~

You can get more verbose output with the -v parameter (or two)
~~~~
get_stop 4He 2MeV -l "7Li0.60 6Li0.40" -t 1000tfu -v -v
~~~~


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


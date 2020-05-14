# JIBAL

Library that provides stopping forces, straggling models, cross sections and various atomic data e.g. masses and abundances for typical IBA applications.

    Copyright (C) 2020 Jaakko Julin

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

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


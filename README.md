# JIBAL

[![DOI](https://zenodo.org/badge/263917531.svg)](https://zenodo.org/badge/latestdoi/263917531)

Library that provides stopping forces, straggling models, cross sections and various atomic data e.g. masses and abundances for typical IBA applications. The GNU General Public License applies to the source code (`*.c` and `*.h` files) and any programs compiled thereof, scripts (`*.sh` and `*.py` files) and the software as a whole, but not to any of the [data](data), since the author of this software can not claim authorship of the data. Please note the exceptions to the GPL license under the following notice.

    Copyright (C) 2020 - 2023 Jaakko Julin

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
This repository includes atomic mass data from the [AME2020 atomic mass evaluation](https://www-nds.iaea.org/amdc/) obtained from the IAEA website. The evaluation has been published in Chinese Physics C 45 (2021) 030002 [(PDF)](https://www-nds.iaea.org/amdc/ame2020/AME2020-a.pdf), 030003 [(PDF)](https://www-nds.iaea.org/amdc/ame2020/AME2020-b.pdf).

This repository includes isotopic abundance data obtained from the CIAAW [website](https://ciaaw.org/isotopic-abundances.htm).

The code includes constants from NIST [CODATA 2018](https://physics.nist.gov/cuu/Constants/).

This library should never be cited in scientific literature without citing the original sources of data. Please contact the [author](AUTHORS) of this library in case of any questions about copyright.

## Installation from sources

Please see the [installation instructions](INSTALL.md).

The latest version from GitHub should compile (at least on Linux), provided that the following badge shows "passing"

![CMake](https://github.com/JYU-IBA/jibal/workflows/CMake/badge.svg)

The versions that are tagged with a version number are considered semi-stable [releases](https://github.com/JYU-IBA/jibal/releases). There is no guarantee of interface compatibility between different versions, at least until a major milestone like v1.0 is reached. Any program compiled against JIBAL should check for version mismatch between compile time and runtime and explicitly specify which version it is intended to work with. This is also one reason why the library interface is not documented (just kidding; the real reason is lack of time).

## Datafiles

The library can not be  used to its full potential without data files. You can obtain some [elsewhere](http://users.jyu.fi/~jaakjuli/jibal/data/). There are data file generators bundled with the package, but they require the use of programs and files which can not be included in the distribution since they are not free software. If you use external data files, please be aware of possible usage restrictions and copyright issues related to distributing them. For scientific use do not cite the JIBAL library, cite the original data.

There is a tool called *jibal_bootstrap* you can use to make a user configuration. See instructions of the data package.

## Usage of bundled tools

### jibaltool

Run from the command line / terminal:

    $ jibaltool

without any arguments to get a short help. You can extract entire files of stopping data and see the current JIBAL configuration.


### Extract stopping data

The jibaltool program can be used to extract stopping like this:
~~~~
jibaltool stop 4He "1 MeV" -l SiO2
~~~~

or for example (stopping in 100 keV steps)
~~~~
jibaltool stop 4He 1MeV 100keV 10MeV -l "Si0.33 O0.33 N0.33" 
~~~~
This should (interpolation and other numerical issues aside) reproduce SRIM 2013 stopping values with the data included in the distribution.


You can also do energy loss calculations in layers, e.g.
~~~~
jibaltool stop 4He 2MeV -l Au -t 1000tfu
~~~~

Elements are assumed to have natural isotopic composition, unless you tell otherwise, e.g.
~~~~
jibaltool stop 4He 2MeV -l "7Li0.60 6Li0.40" -t 1000tfu
~~~~

You can get more verbose output with the -v parameter (or two)
~~~~
jibaltool stop 4He 2MeV -l "7Li0.60 6Li0.40" -t 1000tfu -v -v
~~~~

## Using the JIBAL library with your own programs

Using CMake is preferred, see directory [demo](demo) for an example of a C++ program using JIBAL.

Alternatively when compiling your programs against jibal you can get the compiler flags with pkg-config (assuming pkg-config finds the `jibal.pc` file)

~~~~
pkg-config --cflags --libs jibal
~~~~


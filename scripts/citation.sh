#!/bin/bash
#This script will generate CITATION.cff
citationfile="../CITATION.cff"
versionfile="../version.txt"
today=$(date "+%Y-%m-%d")
read version < "$versionfile"
cat <<END  > "$citationfile"
cff-version: 1.2.0
authors:
 - family-names: "Julin"
   given-names: "Jaakko"
   orcid: "https://orcid.org/0000-0003-4376-891X"
title: "Jyväskylä Ion Beam Analysis Library (JIBAL)"
repository-code: "https://github.com/JYU-IBA/jibal"
license: GPL-2.0-or-later
version: $version
doi: 10.5281/zenodo.5227133
date-released: $today
END

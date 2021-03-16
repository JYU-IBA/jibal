#!/bin/bash
datadir="../data/"

amefile="massround.mas20.txt"
ameurl="https://www-nds.iaea.org/amdc/ame2020/massround.mas20.txt"

#Check number of headers in ame_decoder.c before uncommenting.
#amefile="mass16.txt"
#ameurl="https://www-nds.iaea.org/amdc/ame2016/$amefile"

#Uncomment for 2012 values
#amefile="mass.mas12"
#ameurl="https://www-nds.iaea.org/amdc/ame2012/$amefile"

CC="gcc"
masses_outfile="$datadir/masses.dat"
abundances_outfile="$datadir/abundances.dat"

download_massfile() {
    if [ ! -f "$datadir/$amefile" ]; then
        echo "Downloading $amefile from $ameurl using curl (saving to $datadir/$amefile)."
        curl "$ameurl" > "$datadir/$amefile";
    fi
}

compile_decoder() {
    if [ ! -f "ame_decoder" ]; then
        echo "Compiling ame_decoder with $CC"
        $CC -Wall -DAMEFILE=\"$datadir\/$amefile\" ame_decoder.c -o ame_decoder
    fi
}

convert_massfile() {
    echo "Converting $amefile to $masses_outfile"
    ./ame_decoder|sed -e s/Ed/Nh/ -e s/Ef/Mc/ -e s/Eh/Ts/ -e s/Ei/Og/ > "$masses_outfile"
}

parse_abundances_from_html() {
    ./abundance.py > "$abundances_outfile"
}

do_all() {
    download_massfile;
    compile_decoder;
    convert_massfile;
}


if [ -f "$masses_outfile" ]; then
    read -p "File \"$masses_outfile\" exists. Do you want to regenerate the mass database? (y/N)" reply
    case $reply in
        [Yy]* ) do_all;;
    esac
else
    do_all;
fi

if [ -f "$abundances_outfile" ]; then
    read -p "File \"$abundances_outfile\" exists. Do you want to regenerate the abundances database? (y/N)" reply
    case $reply in
        [Yy]* ) parse_abundances_from_html;;
    esac
else
    parse_abundances_from_html;
fi
exit;

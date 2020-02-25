#!/bin/bash
amefile="mass.mas12"
ameurl="https://www-nds.iaea.org/amdc/ame2012/$amefile"
CC="gcc"
outfile="../data/libiba_masses.dat"

download_massfile() {
    if [ ! -f "$amefile" ]; then
        echo "Downloading $amefile from $ameurl using curl."
        curl -O "$ameurl";
    fi
}

compile_decoder() {
    if [ ! -f "ame_decoder" ]; then
        echo "Compiling ame_decoder with $CC"
        $CC -Wall -DAMEFILE=\"$amefile\" ame_decoder.c -o ame_decoder
    fi
}

convert_massfile() {
    echo "Converting $amefile to $outfile"
    ./ame_decoder > "$outfile"
}

clean_temp_files() {
    rm "$amefile"
}

do_all() {
    download_massfile;
    compile_decoder;
    convert_massfile;
#    clean_temp_files;
}

read -p "Do you want to generate the mass database? (y/N)" reply
case $reply in
    [Yy]* ) do_all;;
esac

exit;

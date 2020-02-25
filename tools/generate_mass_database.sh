#!/bin/bash
amefile="mass16.txt"
ameurl="https://www-nds.iaea.org/amdc/ame2016/$amefile"

#Uncomment for 2012 values
#amefile="mass.mas12"
#ameurl="https://www-nds.iaea.org/amdc/ame2012/$amefile"

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
    echo "Cleaning up."
    rm "$amefile"
    rm "ame_decoder"
}

do_all() {
    download_massfile;
    compile_decoder;
    convert_massfile;
    clean_temp_files;
}


if [ -f "$outfile" ]; then
    read -p "File \"$outfile\" exists. Do you want to regenerate the mass database? (y/N)" reply
    case $reply in
        [Yy]* ) do_all;;
    esac
else
    do_all;
fi

exit;

#!/bin/bash

FT=0

echo Number of commandline: $#
while [[ $# -ne 0 ]]
do
key="$1"

#echo Parsing: $key
case $key in
    FT)
    FT=1
    ;;
    --default)
    DEFAULT=YES
    ;;
    *)
    echo $key is unknown command
    ;;
esac
shift # past argument or value
done

DIR=`pwd`
echo $DIR
echo $FT

rm -rf $DIR/build
mkdir -p $DIR/build
cd $DIR/build

if [ "$FT" -eq "1" ]
then
    cmake -DFT=$FT ..
    make -j8
else
    cmake ..
    make -j8
fi

cp ../*.json ./
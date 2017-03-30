#!/bin/bash
#Usage:
#  stripe_comment.sh file_name.h
#Theory of Operation
#  run c pre processing to get the work done
#Tested on Mac
# 03/29/2017

name=$1.clean.h

sed /\#include/d $1 > $name
cpp -nostdinc $name > $name.tmp
sed /^$/d $name.tmp > $name
sed -i '' s/#.*$// $name
sed -i '' /^\ *$/d $name

rm -fr $name.tmp



#!/bin/bash
#Usage:
#  stripe_comment.sh file_name.h
#Theory of Operation
#  run c pre processing to get the work done
#Tested on Mac
# 03/29/2017

API_LEVEL=26
name=$1.clean.h

sed /\#include/d $1 > $name
cpp -D__ANDROID_API__=$API_LEVEL -nostdinc $name > $name.tmp
sed /^$/d $name.tmp > $name
sed -i '' s/#.*$// $name
sed -i '' /^\ *$/d $name

rm -fr $name.tmp



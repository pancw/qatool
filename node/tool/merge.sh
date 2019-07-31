#!/bin/bash

cd $1
cd light_engine

echo $3
echo $4
./y5$2 -m $3 $4
./y5agent$2 -m $3 $4

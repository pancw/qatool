#!/bin/bash

cd $1
cd light_engine

./y5$2 -s 991 1
./y5agent$2 -s 991 1

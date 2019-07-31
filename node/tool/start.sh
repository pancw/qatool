#!/bin/bash

cd $1
cd light_engine

./$2 >> /dev/null 2>&1 &
#./$2 &

#./y5agent$2 &
#./y5gate$2 &
#./y5backer$2 &
#./y5$2 &
#./y5fight$2 &

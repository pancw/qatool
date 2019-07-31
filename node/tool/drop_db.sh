#!/bin/bash


#/home/qc_test/mongodb/bin/mongo 10.104.119.86:9802/cfwm50_${2} --eval "db.dropDatabase();"
#/home/qc_test/mongodb/bin/mongo 10.104.39.251:9802/cfwm50_${2} --eval "db.dropDatabase();"
#/home/qc_test/mongodb/bin/mongo 10.104.39.251:9802/cfwm50_${2} --eval "db.dropDatabase();"

echo $2:${4}/${3}

/home/qc_test/mongodb/bin/mongo $2:${4}/${3} --eval "db.dropDatabase();"

cd $1
rm -rf log
mkdir log

echo "drop db finish."

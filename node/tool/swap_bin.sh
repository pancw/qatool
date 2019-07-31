#!/bin/bash

cd $1
svn up logic/share_file --accept theirs-conflict

cp logic/share_file/$2 ./light_engine/$3

#cp logic/share_file/y5main ./light_engine/y5$2
#cp logic/share_file/y5agent ./light_engine/y5agent$2
#cp logic/share_file/y5backer ./light_engine/y5backer$2
#cp logic/share_file/y5gate ./light_engine/y5gate$2
#cp logic/share_file/y5fight ./light_engine/y5fight$2

echo $3
echo "cp finish."

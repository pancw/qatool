#!/bin/bash

cd $1
echo $1 $2 $3

rm -rf logic
#svn co svn://111.230.176.208/y5_server_branch/$dir logic
svn co $2 logic --username qc_test1 --password qc_testsgqyz
sed -i "s/^SERVER_ID = .*/SERVER_ID = $3/g;s/^CENTER_ID.*/CENTER_ID = $4/g" logic/module/game.lua

echo "switch done."

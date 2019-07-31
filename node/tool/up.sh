#!/bin/bash

cd $1
svn up logic --accept theirs-conflict --username qc_test1 --password qc_testsgqyz

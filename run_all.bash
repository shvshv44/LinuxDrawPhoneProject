#!/bin/bash

mkdir -p ../logs/
echo "" > ../logs/last_run.txt
./../runs/remote_paint &>> ../logs/last_run.txt


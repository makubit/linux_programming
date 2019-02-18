#!/bin/bash


for i in $(seq 1 $2)
do
       ./k.out -#10 -s1 $1 &
done

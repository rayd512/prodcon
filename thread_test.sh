#!/bin/bash


counter=0
while [ $? -eq 0 ]; do
	./prodcon 20 1 < test.txt
    echo "Iteration $counter"
    counter=$((counter+1))
done
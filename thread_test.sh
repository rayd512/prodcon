#!/bin/bash


counter=0
while [ $? -eq 0 ]; do
	./prodcon 3 $counter < test.txt
    echo "Iteration $counter"
    counter=$((counter+1))
done
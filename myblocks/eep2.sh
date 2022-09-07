#!/usr/bin/env bash

for i in 0 1 2 3
do
    dd if=eep2.bin of=$i.bin bs=1 count=256 skip=$(($i * 256))
done

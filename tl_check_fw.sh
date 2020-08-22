#!/bin/bash
echo "*****************************************************"
echo "this is post build!! current configure is :$1"

echo $1 
echo $2
tc32-elf-objcopy -v -O binary $2.elf $1.bin
../check_fw  $1.bin
echo "**************** end of post build ******************"

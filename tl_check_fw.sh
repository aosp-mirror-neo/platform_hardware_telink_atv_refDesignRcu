#!/bin/bash
echo "*****************************************************"
echo "this is post build!! current configure is :$1"

#echo $1 
#echo $2
tc32-elf-objcopy -v -O binary $2.elf $1.bin
#../check_fw  $1.bin
#../FW_SIG_ENC 3 $1.bin FW_ENC_KEY
#python3 ../ref_sign_fw.py -f $1.bin -p ../priv.pem -k ../FW_ENC_KEY -o $1_enc_sign.bin@
python3 ../ref_sign_fw.py -f $1.bin -p ../PrivateSample.pem -k ../FW_ENC_TEST_KEY -o $1_enc_sign.bin
#py ../ref_sign_fw.py -f $1.bin -p ../PrivateSample.pem -k ../FW_ENC_TEST_KEY -o $1_enc_sign.bin
echo "**************** end of post build ******************"

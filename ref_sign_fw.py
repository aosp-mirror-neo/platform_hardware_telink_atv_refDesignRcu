#!/usr/bin/python
# -*- coding: UTF-8 -*-
import sys, getopt, os
import binascii

from Crypto.Signature import DSS
from Crypto.PublicKey import ECC
from Crypto.Hash import SHA256
from Crypto.Cipher import AES

#from crypto.Hash import SHA256

AES_BLOCK_SIZE        =  16
ENC_OFFSET            =  32
ENC_FLAG              =  0x5A
ENC_INDEX             =  0x01
SIGN_FLAG             =  0x5B
SIGN_INDEX            =  0x01


def fw_set_flags(ifw) :
    with open(ifw, 'rb') as f:
        inputFw_data = f.read()
        f.close()

    # 1. 16-byte alignment, use FF to fill data. 
    # 2. 4-bytes CRC32 data
    fw_fill_size = len(inputFw_data) + 15 - (len(inputFw_data)+15)%16
    fw_size = fw_fill_size + 4

    sign_fw_data = [0xff for i in range(fw_fill_size)]
    common_fw_data = [0xff for i in range(fw_fill_size)]

    for i in range(0,len(inputFw_data)):
        sign_fw_data[i] = inputFw_data[i]
        common_fw_data[i] = inputFw_data[i]

    sign_fw_data[6] = 0x5D
    sign_fw_data[7] = 0x02

    common_fw_data[6] = 0x5D
    common_fw_data[7] = 0x02

    bytes_size = fw_size.to_bytes(4, 'little')   #little
        
    sign_fw_data[0x18] = bytes_size[0]
    sign_fw_data[0x19] = bytes_size[1]
    sign_fw_data[0x1A] = bytes_size[2]
    sign_fw_data[0x1B] = bytes_size[3]

    common_fw_data[0x18] = bytes_size[0]
    common_fw_data[0x19] = bytes_size[1]
    common_fw_data[0x1A] = bytes_size[2]
    common_fw_data[0x1B] = bytes_size[3]

    sign_fw_data[0x1C] = ENC_FLAG
    sign_fw_data[0x1D] = ENC_INDEX
    sign_fw_data[0x1E] = SIGN_FLAG
    sign_fw_data[0x1F] = SIGN_INDEX

    return sign_fw_data, common_fw_data



def fw_sha256_ecdsa(ifwbuf, ikey) :


    # fw_fill_size = len(ifwbuf) + 15 - (len(ifwbuf)+15)%16
    # fw_size = fw_fill_size + 64

    # cfg_fw_data = [0xff for i in range(fw_fill_size)]

    # for i in range(0,len(ifwbuf)):
    #     cfg_fw_data[i] = ifwbuf[i]

    # print(ifwbuf)


    with open(ikey, 'rb') as pem_in:
        priv_key  = ECC.import_key(pem_in.read())
        pem_in.close()

    pub_key =  priv_key.public_key()
    
    signer = DSS.new(priv_key, 'fips-186-3')
    verifier = DSS.new(pub_key, 'fips-186-3')

    # print(priv_key)
    # print(pub_key)

    # print('Private key: 0x%x' % priv_key.export_key())
    # print('Public point (Uncompressed): 0x%s' % pub_key.export_key().hex())

    # print(len(bytes(ifwbuf)))

    hash = SHA256.new(bytes(ifwbuf))

    # print("__hash__", hash.__hash__)

    # print(type(hash))
    # print(hash.__format__)
    hash_hex = bytes.fromhex(hash.hexdigest())

    for i in range(0,99):
        if(i == 99):
            print("signature timeout")
            exit(2)
        signature = signer.sign(hash)
        if(len(signature) != 64):
            continue

        # Verify
        try:
            verifier.verify(hash, signature)
            print('ECDSA Verification OK')
            break
        except ValueError:
            print('ECDSA Verification failed')
            continue

    #signature = priv_key.sign(data, signature_algorithm)
    # print("Sign len:",len(signature),'  Signature: 0x%s' % signature.hex())

    sign_byarray = bytearray.fromhex(signature.hex())

    print("HASH:")
    for i in range (0,len(hash_hex)):
        if i!=0 and i%16==0 :
            print()
        print('0x%02X' %hash_hex[i],end=', ')
    print('\n')
    
    
    print("signature:")
    for i in range (0,len(sign_byarray)):
        if i!=0 and i%16==0 :
            print()
        print('0x%02X' %sign_byarray[i],end=', ')
    print('\n')
    
    # 1. 16-byte alignment, use FF to fill data. 
    # 2. 64-bytes signature
    sign_fw = [0xff for i in range(len(ifwbuf) + 15 - (len(ifwbuf)+15)%16+ 64)]

    # coty ifwbuf to sign_fw
    for i in range(0,len(ifwbuf)):
        sign_fw[i] = ifwbuf[i]

    # print("fw len:",len(ifwbuf))
    # print("sign_fw len:",len(sign_fw))

    # copy 32bytes r
    for i in range(0,len(sign_byarray)):
        sign_fw[len(sign_fw)-64+i] = sign_byarray[i]

    # with open(ofw,'wb') as f:
    #     f.write(bytes(sign_fw))
    #     f.close()
    return sign_fw

def fw_enc_aes(ifwbuf, ikey) :

    # header(32B) and signature(64B) data do not need to be encrypted
    cfg_fw_data = [0xff for i in range(len(ifwbuf) - 32 - 64)]

    for i in range(0,len(cfg_fw_data)):
        cfg_fw_data[i] = ifwbuf[i+32]


    # encryption
    cipher = AES.new(ikey, AES.MODE_ECB)
    ciphertext = cipher.encrypt(bytes(cfg_fw_data))


    enc_fw =  [0xff for i in range(len(ifwbuf))]

    for i in range(0,len(enc_fw)):
        enc_fw[i] = ifwbuf[i]

    for i in range(0,len(ciphertext)):
        enc_fw[i+32] = ciphertext[i]

    return enc_fw

def fw_crc_32(ifwbuf) :
    # print("fw_crc_32")

    input_fw = bytes(ifwbuf)
    # for i in range(0,len(ifwbuf)):
    #     input_fw[i] = ifwbuf[i]

    integer_crc = binascii.crc32(input_fw)
    Invert_integer_crc = integer_crc^0xFFFFFFFF
    # hex_crc = hex(integer_crc)

    # print(hex(integer_crc))
    # print(hex(Invert_integer_crc))

    # bytes_crc = integer_crc.to_bytes(4, 'little')   #little
    Invert_bytes_crc = Invert_integer_crc.to_bytes(4, 'little')

    # print(bytes_crc)
    # print(hex(bytes_crc[0]), hex(bytes_crc[1]), hex(bytes_crc[2]), hex(bytes_crc[3]))

    print("CRC: ", hex(Invert_bytes_crc[0]), hex(Invert_bytes_crc[1]), hex(Invert_bytes_crc[2]), hex(Invert_bytes_crc[3]))

    crc32_fw = [0xff for i in range(len(ifwbuf) + 4)]
    for i in range(0,len(ifwbuf)):
        crc32_fw[i] = ifwbuf[i]


    crc32_fw[len(ifwbuf)+0] = Invert_bytes_crc[0]
    crc32_fw[len(ifwbuf)+1] = Invert_bytes_crc[1]
    crc32_fw[len(ifwbuf)+2] = Invert_bytes_crc[2]
    crc32_fw[len(ifwbuf)+3] = Invert_bytes_crc[3]

    return crc32_fw

def main(argv):
    inputfile = ''
    outputfile = ''
    try:
        opts, args = getopt.getopt(argv,"hf:p:k:o:",["ifw=","iPrivPem=","inputEncKey=","ofw="])    
    except getopt.GetoptError:
        print("except format: py ref_sign_fw.py -f <inputFw> -p <inputPrivPem> -k <inputEncKey> -o <outputFirmware> ")
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print("format: py ref_sign_fw.py -f <inputFw> -p <inputPrivPem> -k <inputEncKey> -o <outputFirmware>")
            sys.exit()
        elif opt in ("-f", "--ifw"):
            inputFw = arg
        elif opt in ("-p", "--iPrivPem"):
            inputPrivPem = arg
        elif opt in ("-k", "--inputEncKey"):
            inputEncKey = arg
        elif opt in ("-o", "--ofw"):
            outputFirmware = arg

    print('input file        : ' + inputFw)
    print('input private key : ' + inputPrivPem)
    print('input AES key     : ' + inputEncKey)
    print('output file name  : ' + outputFirmware)

    
    if os.path.isfile(inputFw) == False:
        print(inputFw, "is not exists")
        sys.exit(2)

    if os.path.isfile(inputPrivPem) == False:
        print(inputPrivPem, "is not exists")
        sys.exit(2)        

    if os.path.isfile(inputEncKey) == False:
        print(inputEncKey, "is not exists")
        sys.exit(2)   

    with open(inputEncKey, 'rb') as f:
        inputEncKey_data = f.read()
        f.close()

    if(len(inputEncKey_data) != 16):
        print("inputEncKey_data length[%d] error!!!" % len(inputEncKey_data) )
        exit(2)

    with open(inputFw, 'rb') as f:
        inputFw_data = f.read()
        f.close()

    sign_flag_buf, common_flag_buf = fw_set_flags(inputFw)

    # sign_crc_buf = fw_crc_32(sign_flag_buf)
    common_crc_buf = fw_crc_32(common_flag_buf)



    # print(len(sign_crc_buf))



    signed_fw_buf = fw_sha256_ecdsa(common_crc_buf, inputPrivPem)

    signed_fw_buf[0x1C] = ENC_FLAG
    signed_fw_buf[0x1D] = ENC_INDEX
    signed_fw_buf[0x1E] = SIGN_FLAG
    signed_fw_buf[0x1F] = SIGN_INDEX

    enc_fw_buf = fw_enc_aes(signed_fw_buf, inputEncKey_data)

    with open(outputFirmware,'wb') as f:
        f.write(bytes(enc_fw_buf))
        f.close()

    with open(inputFw,'wb') as f:
        f.write(bytes(common_crc_buf))
        f.close()

if __name__ == "__main__":
    main(sys.argv[1:])
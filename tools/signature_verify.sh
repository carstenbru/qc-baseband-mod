#!/bin/bash

## Usage: auth.sh IN_FILE [KEYFILE]
##      when called with only IN_FILE parameter (b01 blob for modem or b00 blob for MBA) the signature is verifed
##      when private key file is passed a signature with this key is calculated

TMP_DIR=./auth_dir

#----------------------------------------------------------------------
# preparation and hash calculation


# 0. prepare
if [ $# -eq 0 ]
then
  echo "Usage: auth.sh IN_FILE [KEYFILE]"
  echo -e "\twhen called with only IN_FILE parameter (b01 blob for modem or b00 blob for MBA) the signature is verifed"
  echo -e "\twhen private key file is passed a signature with this key is calculated"
  exit
fi
IN_FILE=$1
mkdir $TMP_DIR 2> /dev/null

if [[ $IN_FILE == *"mba."* ]]
then #80 bytes header
  IMG_SIZE=0x`xxd -e -l 4 -s 28 $IN_FILE | tail -c +11 | head -c 8`
  IMG_DST_PTR=0x`xxd -e -l 4 -s 24 $IN_FILE | tail -c +11 | head -c 8`
  SIG_PTR=0x`xxd -e -l 4 -s 36 $IN_FILE | tail -c +11 | head -c 8`
  SIG_SIZE=0x`xxd -e -l 4 -s 40 $IN_FILE | tail -c +11 | head -c 8`
  SIG_POS=$(( $SIG_PTR - $IMG_DST_PTR + 80 ))
else #40 bytes header
  IMG_SIZE=0x`xxd -e -l 4 -s 16 $IN_FILE | tail -c +11 | head -c 8`
  IMG_DST_PTR=0x`xxd -e -l 4 -s 12 $IN_FILE | tail -c +11 | head -c 8`
  SIG_PTR=0x`xxd -e -l 4 -s 24 $IN_FILE | tail -c +11 | head -c 8`
  SIG_SIZE=0x`xxd -e -l 4 -s 28 $IN_FILE | tail -c +11 | head -c 8`
  SIG_POS=$(( $SIG_PTR - $IMG_DST_PTR + 40 ))
fi

if [ $(( $IMG_SIZE )) -lt $(( $SIG_POS )) ]
then
  echo "error: no signature found in image!"
  exit
fi
SIG_SIZE_INT=$(( $SIG_SIZE ))

# 1. take data to authenticate: hashtable with header (b01 without certs, so ending after hash table)
cat $IN_FILE | head -c $SIG_POS > $TMP_DIR/tosign.bin

# 2. extract SW_ID and MSM_ID
CERTS_POS=`cat $IN_FILE | grep -obaP '\x30\x82..\x30\x82' | grep -o '[0-9]*:' | grep -o '[0-9]*'`
CERTS_POS_ARR=($CERTS_POS)
# assume sign certificate is the first one in the file, this is true for all observed files so ok
dd bs=1 skip=${CERTS_POS_ARR[0]} count=$((${CERTS_POS_ARR[1]} - ${CERTS_POS_ARR[0]})) if=$IN_FILE of=$TMP_DIR/sign_cert.der 2> /dev/null

SW_ID=`cat $TMP_DIR/sign_cert.der | grep -ao '[0-9A-Fa-f]* SW_ID' | grep -o '[0-9A-Fa-f]* '`
SW_ID=0x$SW_ID
echo Using SW_ID=$SW_ID
MSM_ID=`cat $TMP_DIR/sign_cert.der | grep -ao '[0-9A-Fa-f]* HW_ID' | grep -o '[0-9A-Fa-f]* '`
MSM_ID=0x$MSM_ID
echo Using MSM_ID=$MSM_ID

#### HMAC calculation ####

# 3. SHA256 hash on that, prepend ipad=0x3636363636363636 ^ SW_ID (resulting in 40 bytes):
printf '0x%X\n' $(( 0x3636363636363636 ^ $SW_ID )) | xxd -r -p > $TMP_DIR/hash_p1.bin
sha256sum -b $TMP_DIR/tosign.bin | head -c 65 | xxd -r -p >> $TMP_DIR/hash_p1.bin

# 4. SHA256 hash on that, prepend opad=0x5C5C5C5C5C5C5C5C ^ MSM_ID (=HW_ID) (resulting in 40 bytes):
printf '0x%X\n' $(( 0x5C5C5C5C5C5C5C5C ^ $MSM_ID )) | xxd -r -p > $TMP_DIR/hash_p2.bin
sha256sum -b $TMP_DIR/hash_p1.bin | head -c 65 | xxd -r -p >> $TMP_DIR/hash_p2.bin

# 5. SHA256 hash on that
sha256sum -b $TMP_DIR/hash_p2.bin | head -c 65 | xxd -r -p > $TMP_DIR/hash_sig.bin

#----------------------------------------------------------------------
# signature verification

# 6. extract pubkey
openssl x509 -inform der -in $TMP_DIR/sign_cert.der -pubkey -noout > $TMP_DIR/publickey.pem

# 7. extract signature
dd bs=1 skip=$SIG_POS count=$SIG_SIZE_INT if=$IN_FILE of=$TMP_DIR/signature.bin 2> /dev/null

# 8. verify signature
openssl rsautl -verify -inkey $TMP_DIR/publickey.pem -keyform PEM -pubin -in $TMP_DIR/signature.bin > $TMP_DIR/verify.bin

DIFF=`diff $TMP_DIR/verify.bin $TMP_DIR/hash_sig.bin`
echo ---------------------------------
if [ -z "$DIFF" ]
then
  echo "Signature verified successfully"
else
  echo "Verification failed"
fi

if [ -z "$2" ]
then
  exit
fi

#----------------------------------------------------------------------
# signature calculation
openssl rsautl -sign -pkcs -in $TMP_DIR/hash_sig.bin -inkey $2 -out $TMP_DIR/signature_new.bin
echo "calculated a new signature"

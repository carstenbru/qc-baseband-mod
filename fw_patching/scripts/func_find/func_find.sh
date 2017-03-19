#!/bin/bash

#TODO doc!

print_usage() {
  echo "Usage: func_find.sh BASE_FILE TARGET_FW [FUNCTION_NAME/ADDRESS]"
  echo -e "\tBASE_FILE: basis file for which functions are known, can be:"
  echo -e "\t\timage (with wrapper header) (.img), a new wrapper will be generated"
  echo -e "\t\t\tif no function is specified"
  echo -e "\t\tobject file (.o)"
  echo -e "\t\tlibrary file (.lib)" #TODO
  echo -e "\tTARGET_FW: target firmware for which functions should be located, can only be image (.img)"
  echo -e "\tFUNCTION_NAME/ADDRESS: name of function or its address to locate in target firmware,"
  echo -e "\t\tif omitted all possible functions will be searched"
  exit
}

generate_elf() {
    mkdir -p $TMP_DIR
    rm -f $TMP_DIR/modem.*
    mcopy -i $1 ::image/modem.* $TMP_DIR
    python $SEEMOO_FW_PATCH_DIR_ROOT/scripts/image/blob-merge.py $TMP_DIR/modem $TMP_DIR/$2.elf
    rm -f $TMP_DIR/modem.*
}

prepare_base_fw() {
    BASE_FW=$BASE_FILE
    BASE_WRAPPER_HEADER_DIR=${BASE_FW%/*}/
    BASE_WRAPPER_HEADERS=$(find $BASE_WRAPPER_HEADER_DIR/*.h)

    #generate base firmware ELF file
    generate_elf $BASE_FW fw_base

    #generate base firmware symbol table
    make ../build/create_wrapper_lcs.mk $TMP_DIR/fw_wrapper.lcs
}

transfer_wrapper_header() {
    python $SEEMOO_FW_PATCH_DIR_ROOT/scripts/func_find/wrapper_transfer.py $TMP_DIR/fw_base.elf $TMP_DIR/fw_target.elf $TMP_DIR/symtab.json $SED_SCRIPT_FILE
    cp $BASE_WRAPPER_HEADERS $TARGET_WRAPPER_HEADER_DIR
    sed -i -f $SED_SCRIPT_FILE $TARGET_WRAPPER_HEADER_DIR/*.h
}

extract_lib() {
    mkdir $2
    FILE_ABS=$(readlink -e $1)
    START_DIR="$(pwd)"
    cd $2
    ar -x $FILE_ABS
    cd $START_DIR
}

if [ $# -eq 0 ]
then
  print_usage
fi

BASE_FILE=$1

TARGET_FW=$2
TARGET_WRAPPER_HEADER_DIR=${TARGET_FW%/*}/

SEEMOO_FW_PATCH_DIR_ROOT=../../

TMP_DIR=./tmp
SED_SCRIPT_FILE=$TMP_DIR/mapping.sed
export PYTHONPATH=$SEEMOO_FW_PATCH_DIR_ROOT/scripts/utils
export MTOOLS_SKIP_CHECK=1
FW_BASE_DIR=$TMP_DIR

generate_elf $TARGET_FW fw_target

if [[ $BASE_FILE == *".img" ]];then # firmware image as base
    prepare_base_fw
    
    if [ $# -eq 3 ]
    then
        python $SEEMOO_FW_PATCH_DIR_ROOT/scripts/func_find/bin_func_find.py $TMP_DIR/fw_base.elf $TMP_DIR/fw_target.elf $3
    else
        transfer_wrapper_header
    fi
else
    if [[ $BASE_FILE == *".lib" ]];then # library file as base
        EXTRACT_DIR=$TMP_DIR/${BASE_FILE%.lib}/
        extract_lib $BASE_FILE $EXTRACT_DIR
        
        OBJ_FILES=$EXTRACT_DIR/*.o
        for f in $OBJ_FILES
        do
            python $SEEMOO_FW_PATCH_DIR_ROOT/scripts/func_find/func_find_object.py $f $TMP_DIR/fw_target.elf
        done
    else
        if [[ $BASE_FILE == *".o" ]];then # object file as base
            if [ $# -eq 3 ]
            then
                python $SEEMOO_FW_PATCH_DIR_ROOT/scripts/func_find/bin_func_find.py $BASE_FILE $TMP_DIR/fw_target.elf $3
            else
                python $SEEMOO_FW_PATCH_DIR_ROOT/scripts/func_find/func_find_object.py  $BASE_FILE $TMP_DIR/fw_target.elf
            fi
        else
            echo "error: unknown BASE_FILE format"
        fi
    fi
fi

#rm -rf $TMP_DIR

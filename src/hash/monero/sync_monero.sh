
if [ "$1" == "" ]; then
    echo "first parameter must be monero source root"
fi

function copy_file() {
    result="failure"
    cp $1 .
    if [ $? -eq 0 ]; then
        result="success"
    fi
    
    echo coying $1 $result
}

# blake256
copy_file $1/src/crypto/blake256.c
copy_file $1/src/crypto/blake256.h
# groestl
copy_file $1/src/crypto/groestl_tables.h
copy_file $1/src/crypto/groestl.h
copy_file $1/src/crypto/groestl.c
#jh
copy_file $1/src/crypto/jh.c
copy_file $1/src/crypto/jh.h
#keccak
copy_file $1/src/crypto/keccak.c
copy_file $1/src/crypto/keccak.h
#oaes
copy_file $1/src/crypto/oaes_lib.c
copy_file $1/src/crypto/oaes_lib.h
copy_file $1/src/crypto/oaes_config.h
#skein
copy_file $1/src/crypto/skein.c
copy_file $1/src/crypto/skein.h
copy_file $1/src/crypto/skein_port.h
#slow-hash
copy_file $1/src/crypto/slow-hash.c

copy_file $1/src/crypto/aesb.c
copy_file $1/src/crypto/hash.c
copy_file $1/src/crypto/hash-extra-blake.c
copy_file $1/src/crypto/hash-extra-groestl.c
copy_file $1/src/crypto/hash-extra-jh.c
copy_file $1/src/crypto/hash-extra-skein.c

copy_file $1/src/crypto/hash-ops.h
cp $1/src/common/int-util.h ./common/
copy_file $1/contrib/epee/include/warnings.h
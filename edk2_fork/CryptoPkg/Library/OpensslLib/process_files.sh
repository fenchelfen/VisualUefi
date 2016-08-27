#!/bin/sh
#
# This script runs the OpenSSL Configure script, then processes the
# resulting file list into our local OpensslLib.inf and also takes
# a copy of opensslconf.h.
#
# This only needs to be done once by a developer when updating to a
# new version of OpenSSL (or changing options, etc.). Normal users
# do not need to do this, since the results are stored in the EDK2
# git repository for them.

OPENSSL_PATH=$(sed -n '/DEFINE OPENSSL_PATH/{s/.* \(openssl[-0-9.]*[a-z]*\)[[:space:]]*/\1/ p}' OpensslLib.inf)

if ! cd "${OPENSSL_PATH}" ; then
    echo "Cannot change to OpenSSL directory \"${OPENSSL_PATH}\""
    exit 1
fi

./Configure --classic UEFI \
	no-asm \
	no-async \
	no-bf \
	no-camellia \
	no-capieng \
	no-cast \
	no-cms \
	no-ct \
	no-deprecated \
	no-dgram \
	no-dsa \
	no-dynamic-engine \
	no-ec \
	no-engine \
	no-err \
	no-filenames \
	no-hw \
	no-idea \
	no-mdc2 \
	no-poly1305 \
	no-posix-io \
	no-rc2 \
	no-rfc3779 \
	no-ripemd \
	no-scrypt \
	no-sct \
	no-seed \
	no-sock \
	no-srp \
	no-ssl \
	no-stdio \
	no-threads \
	no-ts \
	no-ui \
	no-whirlpool \
    || exit 1

make files
cd -

function filelist ()
{
    echo '1,/# Autogenerated files list starts here/p'
    echo '/# Autogenerated files list ends here/,$p'
    echo '/# Autogenerated files list starts here/a\'

    while read LINE; do
	case "$LINE" in
	    RELATIVE_DIRECTORY=*)
		eval "$LINE"
		;;
	    LIBSRC=*)
		LIBSRC=$(echo "$LINE" | sed s/^LIBSRC=//)
		for FILE in $LIBSRC; do
		    if [ "$FILE" != "b_print.c" ]; then
			echo -e '  $(OPENSSL_PATH)/'$RELATIVE_DIRECTORY/$FILE\\r\\
		    fi
		done
		;;
	esac
    done
    echo -e \\r
}

filelist < "${OPENSSL_PATH}/MINFO" |  sed -n -f - -i OpensslLib.inf

# We can tell Windows users to put this back manually if they can't run
# Configure. For now, until the git repository is fixed to store things
# sanely, also convert to DOS line-endings
unix2dos -n "${OPENSSL_PATH}/include/openssl/opensslconf.h" opensslconf.h

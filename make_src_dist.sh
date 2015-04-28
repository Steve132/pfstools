#/bin/bash
#
# This script is used to create a clean source distribution (starting from 2.0.0)
#

REL=2.0.3

DIR="pfstools-${REL}"
DEST="${DIR}.tgz"

echo "Preparing ${DEST}"

cd ..

# Create a clean copy
rm -rf ${DIR}
rm -rf ${DEST}
cp -r pfstools ${DIR}

# Remove unnecessary files
rm -rf ${DIR}/.git
rm -rf ${DIR}/.nfs*
rm -r ${DIR}/debian
rm -r ${DIR}/build
rm ${DIR}/make_src_dist.sh

# Put into .tgz
tar -czf ${DEST} ${DIR}

# Test
tar -tzf ${DEST} ${DIR}

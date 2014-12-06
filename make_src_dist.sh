#/bin/bash
#
# This script is used to create a clean source distribution (starting from 2.0.0)
#

REL=2.0.0

DIR="pfstools-${REL}"

echo "Preparing ${DIR}.tgz"

cd ..

# Create a clean copy
rm -rf ${DIR}
cp -r pfstools ${DIR}

# Remove unnecessary files
rm -rf ${DIR}/.git
rm -r ${DIR}/debian
rm ${DIR}/make_src_dist.sh

# Put into .tgz
tar -czf ${DIR}.tgz ${DIR}

# Test
tar -tzf ${DIR}.tgz ${DIR}

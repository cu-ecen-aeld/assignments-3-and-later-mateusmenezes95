#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))

# Exporting the variables to be used in make commands
export ARCH=arm64
export CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]; then
    echo "Using default directory ${OUTDIR} for output"
else
    OUTDIR=$(realpath $1)
    echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p "${OUTDIR}"/build

if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
    echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
    git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION} ${OUTDIR}/linux-stable
fi

# Uses 80% of the available cores to build the kernel. At least 1 core is used.
CORES=$(awk 'BEGIN{c=0.8*'$(nproc)'; print (c<1)?1:int(c)}')

if [ ! -e ${OUTDIR}/build/arch/${ARCH}/boot/Image ]; then
    cd ${OUTDIR}/linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    make mrproper O=${OUTDIR}/build
    make defconfig O=${OUTDIR}/build
    make all -j${CORES} O=${OUTDIR}/build
    make modules O=${OUTDIR}/build
    make dtbs O=${OUTDIR}/build
fi
echo "Adding the Image in outdir"
cp ${OUTDIR}/build/arch/${ARCH}/boot/Image ${OUTDIR}/

echo "Creating the staging directory for the root filesystem"
if [ -d "${OUTDIR}/rootfs" ]; then
    echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO(Done): Create necessary base directories
mkdir -p "${OUTDIR}/rootfs"
mkdir -p "${OUTDIR}/rootfs"/{bin,dev,etc,home,lib,lib64,proc,sbin,sys,tmp,usr,var}
mkdir -p "${OUTDIR}/rootfs"/usr/{bin,lib,sbin}
mkdir -p "${OUTDIR}/rootfs"/var/log

if [ ! -d "${OUTDIR}/busybox" ]; then
    git clone git://busybox.net/busybox.git "$OUTDIR/busybox"
    pushd "$OUTDIR/busybox"
    git checkout ${BUSYBOX_VERSION}
    # TODO(done):  Configure busybox
    make distclean
    make defconfig
    popd
fi

# TODO: Make and install busybox
pushd "$OUTDIR/busybox"
make -j${CORES}
make CONFIG_PREFIX="${OUTDIR}/rootfs" install
popd

echo "Library dependencies"
${CROSS_COMPILE}readelf -a "${OUTDIR}/rootfs/bin/busybox" | grep "program interpreter"
${CROSS_COMPILE}readelf -a "${OUTDIR}/rootfs/bin/busybox" | grep "Shared library"

# TODO: Add library dependencies to rootfs
export SYSROOT="$(aarch64-none-linux-gnu-gcc -print-sysroot)"
cp -a "${SYSROOT}"/lib/ld-linux-aarch64.so.1 "${OUTDIR}"/rootfs/lib/
cp -a "${SYSROOT}"/lib64/libm.so.6 "${OUTDIR}"/rootfs/lib64/
cp -a "${SYSROOT}"/lib64/libresolv.so.2 "${OUTDIR}"/rootfs/lib64/
cp -a "${SYSROOT}"/lib64/libc.so.6 "${OUTDIR}"/rootfs/lib64/

# TODO: Make device nodes
sudo mknod -m 666 "${OUTDIR}/rootfs/dev/null" c 1 3
sudo mknod -m 600 "${OUTDIR}/rootfs/dev/console" c 5 1

# TODO: Clean and build the writer utility
pushd "${FINDER_APP_DIR}"
make clean
make CROSS_COMPILE="${CROSS_COMPILE}gcc" writer # Will use the CROSS_COMPILE variable that points to the toolchain
popd

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
cp -a "${FINDER_APP_DIR}/writer" "${OUTDIR}/rootfs/home/"
cp -a "${FINDER_APP_DIR}/finder.sh" "${OUTDIR}/rootfs/home/"
cp -a "${FINDER_APP_DIR}/finder-test.sh" "${OUTDIR}/rootfs/home/"

mkdir -p "${OUTDIR}/rootfs/home/conf"
cp -aL "${FINDER_APP_DIR}/conf"/*.txt "${OUTDIR}/rootfs/home/conf/"

cp -a "${FINDER_APP_DIR}/autorun-qemu.sh" "${OUTDIR}/rootfs/home/"

# TODO: Chown the root directory
sudo chown -R root:root "${OUTDIR}/rootfs"

# TODO: Create initramfs.cpio.gz
cd "${OUTDIR}/rootfs"
find . | cpio -H newc -ov --owner root:root > "../initramfs.cpio"
cd ..
gzip -f initramfs.cpio

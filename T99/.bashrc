export TOPDIR=/data/workspaces/mybuild
. /etc/bashrc
function dot()
{
source $TOPDIR/platform/misc/scripts/devel_funcs
source ~/.bashrc
source ~/devbashrc
}
function hclone()
{
cd $TOPDIR
hg clone http://hg.tintri.com/fs
hg clone http://hg.tintri.com/platform
hg clone http://hg.tintri.com/ui
}
function build_ui0()
{
cd $TOPDIR/ui
./antbuild clean
}
function build_fs()
{
cd $TOPDIR/fs
scons BUILDDIR=$TOPDIR/bld_rs BUILDTYPE=release tree
/auto/savedpackages/bin/import_build_package.py -d ../pkgimport -i imports.yaml
cd $TOPDIR/bld_rs
scons -j 2 # threads = 2 * num cpus works well

# The above command builds the test/ folder as well, which contains the unit tests and takes a while.
# If you're not planning to run unit tests, you can build only txos binaries by doing
scons -j 2 src
}
function build_ui()
{
cd $TOPDIR/ui
./antbuild clean
./antbuild smis-clean
./antbuild
}
function build_os()
{
cd $TOPDIR/platform/os
./build.sh build -j2 #  [-j <threads>]
./build.sh install
cd $TOPDIR/platform/distro/extpkgs/
./build.sh -e build -j2 # [-j <threads>]
./build.sh -e install
}
function build_rpms
{
if [ $1 -eq "-r" ]; then
cd $TOPDIR/platform
sudo distro/tools/mkrel  -f  -b $TOPDIR -r $TOPDIR/bld_rs -u $TOPDIR/ui -e fs -e ui -e util # for rebuild
else
cd $TOPDIR/platform
sudo distro/tools/mkrel  -b $TOPDIR -r $TOPDIR/bld_rs -u $TOPDIR/ui
fi
}

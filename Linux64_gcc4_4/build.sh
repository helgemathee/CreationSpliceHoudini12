#run this script in console with "sh build.sh"

SPLICE_INC=CreationSpliceAPI/src
SPLICE_LIB=CreationSpliceAPI/lib

rm *.o
rm *.a
echo "Removing old files done"

hcustom -L $SPLICE_LIB -I $SPLICE_INC -l CreationSplice-1.9.0_s -l CreationCore-1.9_s src/SOP_Splice.cpp

echo "Compilation done"

#cd /home/ubuntu/houdini12.5/dso






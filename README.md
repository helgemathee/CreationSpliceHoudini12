CreationSpliceHoudini12
=======================
Creation Splice implementation in Houdini 12


Install:

copy "SOP_Splice.so" file and "RT", "Exts" folders to /home/ubuntu/houdini12.5/dso


Compile:
Run console and write:

   export FABRIC_RT_PATH=/home/ubuntu/houdini12.5/dso/RT

   export FABRIC_EXTS_PATH=/home/ubuntu/houdini12.5/dso/Exts

   cd /opt/hfs12.5.469/

   source houdini_setup

   cd < "path to your install folder" /CreationSpliceHoudini12_linux64_gcc4_4>

   sh build.sh

   houdini -foreground



Enjoy it!

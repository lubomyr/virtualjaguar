#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

ln -sf libgl4es.a $LOCAL_PATH/../../../obj/local/$1/libGL.a
#ln -sf libnanogl.a $LOCAL_PATH/../../../obj/local/$1/libGL.a
#ln -sf libglu.a $LOCAL_PATH/../../../obj/local/$1/libGLU.a

JOBS=1

cd virtualjaguar/src/m68000
../../../../setEnvironment-$1.sh sh -c "make -j$JOBS"
cd ../..
#cd virtualjaguar
../../setEnvironment-$1.sh sh -c "make -j$JOBS" && mv -f virtualjaguar ../libapplication-$1.so




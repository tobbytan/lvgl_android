set -e
export ANDROID_HOME=~/Android/nu3001/
export SYSROOT=$ANDROID_HOME/prebuilts/ndk/current/platforms/android-18/arch-arm

export PATH=$PATH:$ANDROID_HOME/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.7/bin/
export CC=arm-linux-androideabi-gcc
export CXX=arm-linux-androideabi-g++

# export PATH=$PATH:$ANDROID_HOME/prebuilts/clang/host/linux-x86/clang-r416183b/bin
# export CC=clang
# export CXX=clang++
#Version=v1.0.3

AOSP_INCLUDES="-I$ANDROID_HOME/frameworks/native/include \
               -I$ANDROID_HOME/system/core/include \
               -I$ANDROID_HOME/hardware/libhardware/include \
               -I$ANDROID_HOME/prebuilts/ndk/current/sources/cxx-stl/system/include" \

export CGO_CFLAGS="--sysroot=$SYSROOT -DANDROID -DLV_CONF_INCLUDE_SIMPLE -I$PROJECT_ROOT/third/lvgl/"
export CGO_CXXFLAGS="--sysroot=$SYSROOT -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES  -DANDROID  $AOSP_INCLUDES"

export CGO_LDFLAGS="--sysroot=$SYSROOT -Landroidlib -lgui -lui -lutils -lbinder"
CGO_ENABLED=1 GOOS=android GOARCH=arm go build  -a -ldflags="-w -s"

adb push dotos /data/local/tmp
adb shell "cd /data/local/tmp; chmod 777 dotos; ./dotos"
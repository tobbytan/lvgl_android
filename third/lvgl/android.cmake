# set(CMAKE_SYSTEM_NAME Android)
# set(CMAKE_ANDROID_API 19)
# set(CMAKE_ANDROID_ARCH_ABI armeabi-v7a)
# set(CMAKE_ANDROID_NDK /home/tobby/Android/android-ndk-r25c)


# set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

# # 如果该文件被当做 C 语言编译，也建议开启 C11（虽然 R"()" 是 C++ 特性）
# set(CMAKE_C_STANDARD 11)
# set_source_files_properties(src/drivers/opengles/assets/lv_opengles_shader.c PROPERTIES LANGUAGE CXX)

# # set(CMAKE_ANDROID_NDK_TOOLCHAIN_VERSION 4.9)
# set(CMAKE_ANDROID_STL_TYPE gnustl_static)


set(CMAKE_SYSTEM_NAME Android)
set(CMAKE_SYSTEM_VERSION 19)

# 明确指定编译器（standalone 路径）
set(CMAKE_C_COMPILER /home/tobby/Android/nu3001//prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.7/bin/arm-linux-androideabi-gcc)
set(CMAKE_CXX_COMPILER /home/tobby/Android/nu3001//prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.7/bin/arm-linux-androideabi-g++)
set_source_files_properties(src/drivers/opengles/assets/lv_opengles_shader.c PROPERTIES LANGUAGE CXX)
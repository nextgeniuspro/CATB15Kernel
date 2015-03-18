================================================================================
Android kernel sources for CAT B15 smartphone (4.1.2)
--------------------------------------------------------------------------------
![](https://github.com/yevgeniy-logachev/CATB15Kernel/blob/master/splash.jpg)

================================================================================
Requirements
--------------------------------------------------------------------------------
- Ubuntu 14.04 (64 bit)
- Ram 2Gb+
- HDD 30Gb+
- NDK
 Install additional package:
 sudo apt-get install libc6:i386 libncurses5:i386 libstdc++6:i386 libncurses5-dev

================================================================================
Build kernel
--------------------------------------------------------------------------------
- cd CATB15KernelPath/kernel
- export CROSS_COMPILE=NDKPath/toolchains/arm-linux-androideabi-4.6/prebuilt/linux-x86_64/bin/arm-linux-androideabi-
- TARGET_PRODUCT=vt58 MTK_ROOT_CUSTOM=../mediatek/custom/ MTK_PATH_SOURCE=../mediatek/kernel/ MTK_PATH_PLATFORM=../mediatek/platform/mt6577/kernel/ ARCH_MTK_PLATFORM=mt6577 TARGET_BUILD_VARIANT=user ./build.sh clean
- TARGET_PRODUCT=vt58 MTK_ROOT_CUSTOM=../mediatek/custom/ MTK_PATH_SOURCE=../mediatek/kernel/ MTK_PATH_PLATFORM=../mediatek/platform/mt6577/kernel/ ARCH_MTK_PLATFORM=mt6577 TARGET_BUILD_VARIANT=user ./build.sh release

After compilation kernel binary will be placed in CATB15KernelPath/mediatek/custom/out

================================================================================
Build boot.img
--------------------------------------------------------------------------------
- Place kernel_vt58.bin kernel binary to CATB15KernelPath/bootimg
- run ./repack-MT65xx.pl -boot kernel_vt58.bin ramdisk boot.img
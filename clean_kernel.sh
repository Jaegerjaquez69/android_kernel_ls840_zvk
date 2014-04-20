export PATH=~/zvk-ls840/android/prebuilt/linux-x86/toolchain/arm-eabi-4.4.3/bin:$PATH
rm -rf ./out
make ARCH=arm CROSS_COMPILE=arm-eabi- clean &&  make mrproper

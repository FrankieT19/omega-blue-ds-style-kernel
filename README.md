#EZ-FLASH  Omega Definitive Edition BLUE Kernel

### How to build 

    1.We use devkitARM_r53, you can use the current version or newer.
    2.Set the following environment variables in system, or modify the value in build.bat, based on your installation path
 
        PATH,DEVKITARM,DEVKITPRO,LIBGBA

    3.Double click on build.bat under winodws. If it goes well, you will get ezbluekernel.gba
    4.Double click on  Link_kernel_image.exe link the ezbluekernel.gba and image.bin to ezbluekernel.bin, that is the omega  kernel upgrade file
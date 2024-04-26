# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/Softwares/Espressif/frameworks/esp-idf-v5.1.2/components/bootloader/subproject"
  "D:/Learning/DoAn1/esp32_device_mm/build/bootloader"
  "D:/Learning/DoAn1/esp32_device_mm/build/bootloader-prefix"
  "D:/Learning/DoAn1/esp32_device_mm/build/bootloader-prefix/tmp"
  "D:/Learning/DoAn1/esp32_device_mm/build/bootloader-prefix/src/bootloader-stamp"
  "D:/Learning/DoAn1/esp32_device_mm/build/bootloader-prefix/src"
  "D:/Learning/DoAn1/esp32_device_mm/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/Learning/DoAn1/esp32_device_mm/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/Learning/DoAn1/esp32_device_mm/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()

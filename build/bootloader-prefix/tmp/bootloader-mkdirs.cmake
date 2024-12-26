# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/yjlam/esp/v5.3.1/esp-idf/components/bootloader/subproject"
  "D:/!MEC3831/!!WirelessDataSystem/Vehicle_v1/build/bootloader"
  "D:/!MEC3831/!!WirelessDataSystem/Vehicle_v1/build/bootloader-prefix"
  "D:/!MEC3831/!!WirelessDataSystem/Vehicle_v1/build/bootloader-prefix/tmp"
  "D:/!MEC3831/!!WirelessDataSystem/Vehicle_v1/build/bootloader-prefix/src/bootloader-stamp"
  "D:/!MEC3831/!!WirelessDataSystem/Vehicle_v1/build/bootloader-prefix/src"
  "D:/!MEC3831/!!WirelessDataSystem/Vehicle_v1/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/!MEC3831/!!WirelessDataSystem/Vehicle_v1/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/!MEC3831/!!WirelessDataSystem/Vehicle_v1/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()

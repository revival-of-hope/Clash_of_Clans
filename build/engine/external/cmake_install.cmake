# Install script for directory: F:/code store/game test/Clash_of_Clans/cocos2d/external

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/Clash_of_Clans")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/Box2D/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/chipmunk/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/freetype2/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/recast/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/bullet/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/jpeg/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/openssl/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/uv/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/webp/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/websockets/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/tinyxml2/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/xxhash/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/xxtea/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/clipper/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/edtaa3func/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/ConvertUTF/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/poly2tri/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/md5/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/curl/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/png/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/win32-specific/gles/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/win32-specific/icon/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/win32-specific/MP3Decoder/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/win32-specific/OggDecoder/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/win32-specific/OpenalSoft/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/glfw3/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/zlib/cmake_install.cmake")
  include("F:/code store/game test/Clash_of_Clans/build/engine/external/unzip/cmake_install.cmake")

endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "F:/code store/game test/Clash_of_Clans/build/engine/external/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()

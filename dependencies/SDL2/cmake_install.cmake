# Install script for directory: D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/SDL2")
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

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/SDL2/Debug/SDL2.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/SDL2/Release/SDL2.lib")
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/SDL2/Debug/SDL2.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/SDL2/Release/SDL2.lib")
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/SDL2/Debug/SDL2.dll")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/SDL2/Release/SDL2.dll")
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/SDL2/Debug/SDL2main.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/SDL2/Release/SDL2main.lib")
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/SDL2" TYPE FILE FILES
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/begin_code.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/close_code.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_assert.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_atomic.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_audio.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_bits.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_blendmode.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_clipboard.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_config_android.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_config_iphoneos.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_config_macosx.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_config_minimal.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_config_pandora.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_config_psp.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_config_windows.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_config_winrt.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_config_wiz.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_copying.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_cpuinfo.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_egl.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_endian.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_error.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_events.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_filesystem.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_gamecontroller.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_gesture.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_haptic.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_hints.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_joystick.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_keyboard.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_keycode.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_loadso.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_log.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_main.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_messagebox.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_mouse.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_mutex.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_name.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_opengl.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_opengles.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_opengles2.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_opengles2_gl2.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_opengles2_gl2ext.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_opengles2_gl2platform.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_opengles2_khrplatform.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_opengl_glext.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_pixels.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_platform.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_power.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_quit.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_rect.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_render.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_revision.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_rwops.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_scancode.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_shape.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_stdinc.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_surface.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_system.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_syswm.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_test.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_test_assert.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_test_common.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_test_compare.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_test_crc32.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_test_font.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_test_fuzzer.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_test_harness.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_test_images.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_test_log.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_test_md5.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_test_random.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_thread.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_timer.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_touch.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_types.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_version.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/include/SDL2/include/SDL_video.h"
    "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/SDL2/include/SDL_config.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "D:/Users/Flynn/Documents/Visual Studio 2015/Projects/Basilisk/dependencies/SDL2/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")

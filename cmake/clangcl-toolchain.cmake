set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR AMD64)
set(XWIN $ENV{XWIN})

set(CMAKE_C_COMPILER   /usr/bin/clang-cl-18)
set(CMAKE_CXX_COMPILER /usr/bin/clang-cl-18)
set(CMAKE_LINKER       /usr/bin/lld-link)
set(CMAKE_AR           /usr/bin/llvm-lib)
set(CMAKE_RC_COMPILER  /usr/bin/llvm-rc)
set(CMAKE_MT           /usr/bin/llvm-mt)

set(CMAKE_C_COMPILER_TARGET   x86_64-pc-windows-msvc)
set(CMAKE_CXX_COMPILER_TARGET x86_64-pc-windows-msvc)

set(_inc "/imsvc ${XWIN}/crt/include /imsvc ${XWIN}/sdk/include/ucrt /imsvc ${XWIN}/sdk/include/um /imsvc ${XWIN}/sdk/include/shared /imsvc ${XWIN}/sdk/include/winrt")
set(CMAKE_C_FLAGS_INIT   "--target=x86_64-pc-windows-msvc -fms-compatibility-version=19.38 -D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH -D_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR ${_inc}")
set(CMAKE_CXX_FLAGS_INIT "--target=x86_64-pc-windows-msvc -fms-compatibility-version=19.38 /EHsc -D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH -D_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR ${_inc}")

set(_lib "/libpath:${XWIN}/crt/lib/x86_64 /libpath:${XWIN}/sdk/lib/um/x86_64 /libpath:${XWIN}/sdk/lib/ucrt/x86_64")
set(CMAKE_EXE_LINKER_FLAGS_INIT    "${_lib}")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${_lib}")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "${_lib}")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

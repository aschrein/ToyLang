project(llvmTest)

find_package(LLVM REQUIRED)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}:")
message(STATUS "   - CMake module path: ${LLVM_CMAKE_DIR}")
message(STATUS "   - Include path     : ${LLVM_INCLUDE_DIRS}")
message(STATUS "   - Binary path      : ${LLVM_TOOLS_BINARY_DIR}")


list(APPEND CMAKE_MODULE_PATH ${LLVM_CMAKE_DIR})


include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(llvmTest main.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -fno-rtti -fvisibility-inlines-hidden")
target_link_libraries(llvmTest PRIVATE
LLVMX86AsmParser
LLVMX86AsmPrinter
LLVMX86CodeGen
LLVMX86Desc
LLVMX86Disassembler
LLVMX86Info
LLVMX86Utils  
LLVMMCJIT
LLVMProfileData
LLVMSupport
LLVMMCParser
LLVMMC
LLVMBitReader
LLVMOption
LLVMCore)

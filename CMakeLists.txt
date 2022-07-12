cmake_minimum_required(VERSION 3.10)

project(rouflex)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra")

############################################
#   THREADS LIBRARY
############################################

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

############################################
#   TIMEPERIOD LIBRARY
############################################
add_library(timeperiod
    include/rouflex/timeperiod.h
    src/timeperiod/timeperiod.cpp
)

target_include_directories(timeperiod
    PUBLIC include
    PRIVATE src
)

############################################
#   NLOHMANN'S JSON LIBRARY
############################################

# Typically you don't care so much for a third party library's tests to be
# run from your own project's code.
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(third_party/json)

############################################
#   INSTANCE LIBRARY
############################################
add_library(instance
    include/rouflex/instance.h
    src/instance/instance.cpp
)

set_target_properties(instance PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(instance
    PRIVATE nlohmann_json::nlohmann_json
)

############################################
#   ENCODING LIBRARY
############################################
add_library(encoding
    include/rouflex/encoding.h
    src/encoding/encoding.cpp
)

target_include_directories(encoding
    PUBLIC include
    PRIVATE src
)

target_link_libraries(encoding
    PUBLIC instance
)

############################################
#   DECODER LIBRARY
############################################
add_library(decoder
    include/rouflex/decoder.h
    src/decoder/decoder.cpp
)

target_include_directories(decoder
    PUBLIC include
    PRIVATE src
)

target_link_libraries(decoder
    PUBLIC timeperiod
    PUBLIC instance
    PUBLIC encoding
)

############################################
#   ABC LIBRARY
############################################
add_library(abc
    include/rouflex/abc.h
    src/abc/abc.cpp
)

target_include_directories(abc
    PUBLIC include
    PRIVATE src
)

target_link_libraries(abc
    PUBLIC instance
    PUBLIC encoding
    PUBLIC decoder
    PUBLIC Threads::Threads
)

############################################
#   SL-ABC LIBRARY
############################################
add_library(slabc
    include/rouflex/slabc.h
    src/slabc/slabc.cpp
)

target_include_directories(slabc
    PUBLIC include
    PRIVATE src
)

target_link_libraries(slabc
    PUBLIC instance
    PUBLIC encoding
    PUBLIC decoder
    PUBLIC Threads::Threads
)

############################################
#   SL-ABC2 LIBRARY
############################################
add_library(slabc2
    include/rouflex/slabc2.h
    src/slabc2/slabc2.cpp
)

target_include_directories(slabc2
    PUBLIC include
    PRIVATE src
)

target_link_libraries(slabc2
    PUBLIC instance
    PUBLIC encoding
    PUBLIC decoder
    PUBLIC Threads::Threads
)

############################################
#   ExecSLABC2 LIBRARY
############################################
add_library(exec_slabc
    include/rouflex/exec_slabc.h
    src/exec_slabc/exec_slabc.cpp
)

target_include_directories(exec_slabc
    PUBLIC include
    PRIVATE src
)

target_link_libraries(exec_slabc
    PUBLIC instance
    PUBLIC slabc2
    PUBLIC Threads::Threads
)

############################################
#   SA LIBRARY
############################################
add_library(sa
    include/rouflex/sa.h
    src/sa/sa.cpp
)

target_include_directories(sa
    PUBLIC include
    PRIVATE src
)

target_link_libraries(sa
    PUBLIC instance
    PUBLIC encoding
    PUBLIC decoder
)

############################################
#   SLGA LIBRARY
############################################
add_library(slga
    include/rouflex/slga.h
    src/slga/slga.cpp
)

target_include_directories(slga
    PUBLIC include
    PRIVATE src
)

target_link_libraries(slga
    PUBLIC instance
    PUBLIC encoding
    PUBLIC decoder
    PRIVATE slabc2
)

############################################
#   CHSLABC LIBRARY
############################################
add_library(chslabc
    include/rouflex/chslabc.h
    src/chslabc/chslabc.cpp
)

target_include_directories(chslabc
    PUBLIC include
    PRIVATE src
)

target_link_libraries(chslabc
    PUBLIC instance
    PUBLIC encoding
    PUBLIC decoder
    PRIVATE slabc2
)

############################################
#   TS LIBRARY
############################################
add_library(ts
    include/rouflex/ts.h
    src/ts/ts.cpp
)

target_include_directories(ts
    PUBLIC include
    PRIVATE src
)

target_link_libraries(ts
    PUBLIC instance
    PUBLIC encoding
    PUBLIC decoder
)

############################################
#   GTEST STUFF
############################################
set (gtest_force_shared_crt ON CACHE BOOL "MSVC defaults to shared CRT" FORCE)
add_subdirectory(third_party/googletest)
target_compile_definitions(gtest
    PUBLIC
        GTEST_LANG_CXX20
        GTEST_HAS_TR1_TUPLE=0
)

############################################
#   TESTS
############################################
add_executable(tests
    tests/timeperiod_test.cpp
    tests/instance_test.cpp
    tests/encoding_test.cpp
    tests/abc_test.cpp
    tests/slabc_test.cpp
)

#set_target_properties(tests PROPERTIES COMPILE_FLAGS "$cxx_strict")
target_link_libraries(tests
    gtest
    gtest_main
    timeperiod
    instance
    encoding
    abc
    slabc
)



############################################
#   EXECUTABLES CODE
############################################
add_executable(mut_and_gantt
    execs/mut_and_gantt.cpp
)

#set_target_properties(tests PROPERTIES COMPILE_FLAGS "$cxx_strict")
target_link_libraries(mut_and_gantt
    instance
    encoding
    decoder
)

add_executable(select_phyb_init
    execs/select_phyb_init.cpp
)

#set_target_properties(tests PROPERTIES COMPILE_FLAGS "$cxx_strict")
target_link_libraries(select_phyb_init
    instance
    encoding
    decoder
)

add_executable(run_slabc2
    execs/run_slabc2.cpp
)

target_link_libraries(run_slabc2
    instance
    exec_slabc
    Threads::Threads
)

add_executable(show_instances
    execs/show_instances.cpp
)

target_link_libraries(show_instances
    instance
)

add_executable(execute_sa
    execs/execute_sa.cpp
)

target_link_libraries(execute_sa
    instance
    encoding
    decoder
    sa
)

add_executable(execute_abc
    execs/execute_abc.cpp
)

target_link_libraries(execute_abc
    instance
    abc
)

add_executable(execute_slga
    execs/execute_slga.cpp
)

target_link_libraries(execute_slga
    instance
    slga
)

add_executable(run_slga
    execs/run_slga.cpp
)

target_link_libraries(run_slga
    instance
    slga
    Threads::Threads
)

add_executable(run_ts
    execs/run_ts.cpp
)

target_link_libraries(run_ts
    instance
    ts
    Threads::Threads
)

############################################
#   MAIN CODE
############################################
add_executable(main
    main/main.cpp
)

#set_target_properties(tests PROPERTIES COMPILE_FLAGS "$cxx_strict")
target_link_libraries(main
    instance
    encoding
    decoder
    abc
    slabc2
    #Threads::Threads
)

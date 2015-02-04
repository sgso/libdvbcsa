include_directories(../src ../)

if(CMAKE_BUILD_TYPE MATCHES RELEASE)
  messsage("Release build")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -fomit-frame-pointer")
endif()

add_executable(benchenc benchenc.c)
target_link_libraries(benchenc dvbcsa)

add_executable(benchdec benchdec.c)
target_link_libraries(benchdec dvbcsa)

add_executable(testenc testenc.c)
target_link_libraries(testenc dvbcsa)

add_executable(testdec testdec.c)
target_link_libraries(testdec dvbcsa)

add_executable(testbsops testbsops.c)
target_link_libraries(testbsops dvbcsa)

add_executable(benchbitslice benchbitslice.c)
target_link_libraries(benchbitslice dvbcsa)

add_executable(benchbitsliceks benchbitsliceks.c)
target_link_libraries(benchbitsliceks dvbcsa)

add_executable(testbitslice testbitslice.c)
target_link_libraries(testbitslice dvbcsa)

add_executable(benchks benchks.c)
target_link_libraries(benchks dvbcsa)

include_directories(${PROJECT_SOURCE_DIR}/include)
set(LIB_SRC vlstp.c
            timer.c
            bitops.c
            bitmap.c
            tsnmap.c
            socket_layer.c )
add_library(vlstp SHARED ${LIB_SRC})
target_link_libraries(vlstp pthread )
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

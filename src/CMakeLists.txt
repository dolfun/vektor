set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_library(image image.h image.cc)
target_compile_features(image PRIVATE cxx_std_23)
target_link_libraries(image PRIVATE stb)

add_executable(main main.cc)
target_compile_features(main PRIVATE cxx_std_23)
target_link_libraries(main PRIVATE image)
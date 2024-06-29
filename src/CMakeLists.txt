set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_library(image image.h kernel.h)
target_compile_features(image PRIVATE cxx_std_23)
target_link_libraries(image PUBLIC glm::glm)

add_library(image_io image_io.h image_io.cc)
target_compile_features(image_io PRIVATE cxx_std_23)
target_link_libraries(image_io PUBLIC image PRIVATE stb)

add_library(canny_edge_detector canny_edge_detector.h canny_edge_detector.cc)
target_compile_features(canny_edge_detector PRIVATE cxx_std_23)
target_link_libraries(canny_edge_detector PUBLIC image)

add_library(tracer tracer.h tracer.cc bezier_curve.h)
target_compile_features(tracer PRIVATE cxx_std_23)
target_link_libraries(tracer PUBLIC image)

add_library(renderer renderer.h renderer.cc bezier_curve.h)
target_compile_features(renderer PRIVATE cxx_std_23)
target_link_libraries(renderer PUBLIC image)

add_executable(vektor main.cc)
target_compile_features(vektor PRIVATE cxx_std_23)
target_link_libraries(vektor PRIVATE canny_edge_detector tracer renderer image_io)
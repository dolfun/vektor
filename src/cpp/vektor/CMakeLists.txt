add_library(image image.h kernel.h)
target_compile_features(image PUBLIC cxx_std_23)
target_link_libraries(image PUBLIC glm::glm)

add_library(image_io image_io.h image_io.cc)
target_compile_features(image_io PUBLIC cxx_std_23)
target_link_libraries(image_io PUBLIC image PRIVATE stb)

add_library(canny_edge_detector canny_edge_detector.h canny_edge_detector.cc)
target_compile_features(canny_edge_detector PUBLIC cxx_std_23)
target_link_libraries(canny_edge_detector PUBLIC image)

add_library(tracer tracer.h tracer.cc bezier_curve.h)
target_compile_features(tracer PUBLIC cxx_std_23)
target_link_libraries(tracer PUBLIC image)

add_library(renderer renderer.h renderer.cc bezier_curve.h)
target_compile_features(renderer PUBLIC cxx_std_23)
target_link_libraries(renderer PUBLIC image)

add_library(vektor_lib INTERFACE)
target_link_libraries(vektor_lib INTERFACE image image_io canny_edge_detector tracer renderer)
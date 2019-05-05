cmake_minimum_required (VERSION 2.6)

add_compile_options(-std=c++11)

add_compile_options(-g)

include_directories(/home/andy/workspace/arcface/inc)
link_directories(/home/andy/workspace/arcface/lib/linux_x64 /usr/local/lib)

project (arcsoft_afd_cv)

find_package(OpenCV 3 REQUIRED)

add_executable(arcsoft_afd_cv arcsoft_resource.cpp)
target_link_libraries(arcsoft_afd_cv -lhiredis -lmysqlclient -larcsoft_fsdk_face_detection -lpthread -larcsoft_fsdk_face_recognition -larcsoft_fsdk_age_estimation -larcsoft_fsdk_gender_estimation ${OpenCV_LIBS})
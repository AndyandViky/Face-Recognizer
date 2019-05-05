cmake_minimum_required (VERSION 2.6)

add_compile_options(-std=c++11)

add_compile_options(-g)

include_directories(/home/yanglin/yl/c++/arcsoft-arcface/arcface/inc)
link_directories(/home/yanglin/yl/c++/arcsoft-arcface/arcface/lib/linux_x64 /usr/local/lib)

project (arcsoft_afd_cv)

add_executable(arcsoft_afd_cv arcsoft_resource.cpp)
target_link_libraries(arcsoft_afd_cv -lhiredis -lmysqlclient -larcsoft_fsdk_face_detection -lpthread -larcsoft_fsdk_face_recognition -larcsoft_fsdk_age_estimation -larcsoft_fsdk_gender_estimation -lopencv_core -lopencv_highgui -lopencv_imgproc)

cmake_minimum_required(VERSION 3.0)
set(CMAKE_CURRENT_BINARY_DIR, ./buildx)
project(os-afs-server)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Boost REQUIRED COMPONENTS system)

add_custom_target(
   kernel ALL
   COMMAND make
   WORKING_DIRECTORY ${os-afs-server_SOURCE_DIR}/kernel
)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS}) 
    add_executable(afs_server afs_server.cpp) 
    target_link_libraries(afs_server ${Boost_LIBRARIES})

    add_executable(afs_test_client afs_test_client.cpp) 
    target_link_libraries(afs_test_client ${Boost_LIBRARIES})

    
endif()
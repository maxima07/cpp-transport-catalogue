cmake_minimum_required(VERSION 3.11)
project(transport-catalogue CXX)

if (CMAKE_SYSTEM_NAME MATCHES "^MINGW")
    set(SYSTEM_LIBS -lstdc++)
else()
    set(SYSTEM_LIBS)
endif()

set(HEADER domain.h geo.h graph.h ranges.h json_builder.h json_reader.h json.h map_renderer.h request_handler.h router.h svg.h transport_router.h transport_catalogue.h)
set(REALIZ domain.cpp geo.cpp json_builder.cpp json_reader.cpp json.cpp map_renderer.cpp request_handler.cpp svg.cpp transport_router.cpp transport_catalogue.cpp)

add_executable(transport-catalogue main.cpp ${HEADER} ${REALIZ})
 
 

 

 
 
 

 
 
                           
                               
                           
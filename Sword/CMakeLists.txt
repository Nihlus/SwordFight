cmake_minimum_required(VERSION 3.6)
project(Sword)

set(Sword_VERSION_MAJOR 1)
set(Sword_VERSION_MINOR 0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_compile_options(-std=gnu++14 -Wno-c++11-narrowing)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/cmake_modules" ${CMAKE_MODULE_PATH})

find_package(OpenGL)
find_package(OpenCL)
find_package(GLEW)
find_package(Threads)
find_package(SFML 2 REQUIRED system window graphics network audio)

set(SWORD_SOURCES
		${PROJECT_SOURCE_DIR}/cape.cpp
		${PROJECT_SOURCE_DIR}/fighter.cpp
		${PROJECT_SOURCE_DIR}/game_state_manager.cpp
		${PROJECT_SOURCE_DIR}/main.cpp
		${PROJECT_SOURCE_DIR}/map_tools.cpp
		${PROJECT_SOURCE_DIR}/menu.cpp
		${PROJECT_SOURCE_DIR}/music.cpp
		${PROJECT_SOURCE_DIR}/object_cube.cpp
		${PROJECT_SOURCE_DIR}/particle_effect.cpp
		${PROJECT_SOURCE_DIR}/physics.cpp
		${PROJECT_SOURCE_DIR}/server_networking.cpp
		${PROJECT_SOURCE_DIR}/sound.cpp
		${PROJECT_SOURCE_DIR}/ui_manager.cpp
		${PROJECT_SOURCE_DIR}/text.cpp)

set(SWORD_HEADERS
		${PROJECT_SOURCE_DIR}/bbox.hpp
		${PROJECT_SOURCE_DIR}/cape.hpp
		${PROJECT_SOURCE_DIR}/colour_defines.hpp
		${PROJECT_SOURCE_DIR}/fighter.hpp
		${PROJECT_SOURCE_DIR}/game_state_manager.hpp
		${PROJECT_SOURCE_DIR}/map_tools.hpp
		${PROJECT_SOURCE_DIR}/menu.hpp
		${PROJECT_SOURCE_DIR}/music.hpp
		${PROJECT_SOURCE_DIR}/object_cube.hpp
		${PROJECT_SOURCE_DIR}/particle_effect.hpp
		${PROJECT_SOURCE_DIR}/physics.hpp
		${PROJECT_SOURCE_DIR}/server_networking.hpp
		${PROJECT_SOURCE_DIR}/sound.hpp
		${PROJECT_SOURCE_DIR}/text.hpp
		${PROJECT_SOURCE_DIR}/ui_manager.hpp
		${PROJECT_SOURCE_DIR}/util.hpp)

set(RENDERER_SOURCES
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/Game/Galaxy/galaxy.cpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/Game/space_manager.cpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/clstate.cpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/controls.cpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/engine.cpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/goo.cpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/hologram.cpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/interact_manager.cpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/light.cpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/logging.cpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/network.cpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/obj_info_struct.cpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/obj_load.cpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/object.cpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/object_context.cpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/objects_container.cpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/ocl.cpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/point_cloud.cpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/projectile_buffer_gen.cpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/settings_loader.cpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/text_handler.cpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/texture.cpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/texture_context.cpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/triangle.cpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/ui_manager.cpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/vertex.cpp)

set(RENDERER_HEADERS
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/Game/Galaxy/galaxy.hpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/Game/space_manager.hpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/clstate.h
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/controls.hpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/engine.hpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/goo.hpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/hologram.hpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/interact_manager.hpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/light.hpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/logging.hpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/network.hpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/obj_g_descriptor.hpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/obj_info_struct.hpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/obj_load.hpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/object.hpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/object_context.hpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/objects_container.hpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/ocl.h
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/point_cloud.hpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/proj.hpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/projectile_buffer_gen.hpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/settings_loader.hpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/text_handler.hpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/texture.hpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/texture_context.hpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/triangle.hpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/ui_manager.hpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/vec.hpp
		${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/vertex.hpp)

set(SERVER_SOURCES
		${PROJECT_SOURCE_DIR}/../sword_server/game_server/game_modes.cpp
		${PROJECT_SOURCE_DIR}/../sword_server/reliability_shared.cpp
		${PROJECT_SOURCE_DIR}/../sword_server/teaminfo_shared.cpp)

set(SERVER_HEADERS
		${PROJECT_SOURCE_DIR}/../sword_server/game_server/game_modes.hpp
		${PROJECT_SOURCE_DIR}/../sword_server/reliability_shared.hpp
		${PROJECT_SOURCE_DIR}/../sword_server/teaminfo_shared.hpp)

set(IMGUI_SOURCES
		${PROJECT_SOURCE_DIR}/../../libraries/imgui/imgui.cpp
		${PROJECT_SOURCE_DIR}/../../libraries/imgui/imgui_draw.cpp
		${PROJECT_SOURCE_DIR}/../../libraries/imgui-sfml/imgui-SFML.cpp)

set(IMGUI_HEADERS
		${PROJECT_SOURCE_DIR}/../../libraries/imgui/imgui.h
		${PROJECT_SOURCE_DIR}/../../libraries/imgui-sfml/imgui-SFML.h)

include_directories(
		${PROJECT_SOURCE_DIR}/
		${PROJECT_SOURCE_DIR}/../../libraries/
		${PROJECT_SOURCE_DIR}/../../libraries/imgui
		${PROJECT_SOURCE_DIR}/../../libraries/imgui-sfml
		${PROJECT_SOURCE_DIR}/../../libraries/compute/include
)

add_executable(${PROJECT_NAME} ${SWORD_SOURCES} ${SWORD_HEADERS} ${SERVER_SOURCES} ${SERVER_HEADERS} ${RENDERER_SOURCES} ${RENDERER_HEADERS} ${IMGUI_SOURCES} ${IMGUI_HEADERS})

target_link_libraries(
		${PROJECT_NAME}
		${OPENGL_gl_LIBRARY}
		${OpenCL_LIBRARIES}
		${CMAKE_THREAD_LIBS_INIT}
		${SFML_LIBRARIES})

file(COPY ${PROJECT_SOURCE_DIR}/../../OpenCLRenderer/cl2.cl DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
cmake_minimum_required(VERSION 3.6)
project(Sword)

set(Sword_VERSION_MAJOR 1)
set(Sword_VERSION_MINOR 0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

set(SWORD_EXTERNAL_PROJECTS_PATH "${PROJECT_SOURCE_DIR}/../lib")

add_compile_options(-std=gnu++14 -Wno-c++11-narrowing -fexceptions -DCL_USE_DEPRECATED_OPENCL_1_1_APIS)
set(CMAKE_MODULE_PATH "${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/cmake_modules" ${CMAKE_MODULE_PATH})

find_package(OpenGL)
find_package(OpenCL)
find_package(GLEW)
find_package(Threads)
find_package(SFML 2 REQUIRED system window graphics audio)

set(SWORD_SOURCES
		${PROJECT_SOURCE_DIR}/cape.cpp
		${PROJECT_SOURCE_DIR}/fighter.cpp
		${PROJECT_SOURCE_DIR}/game_state_manager.cpp
		${PROJECT_SOURCE_DIR}/main.cpp
		${PROJECT_SOURCE_DIR}/map_tools.cpp
		${PROJECT_SOURCE_DIR}/menu.cpp
		${PROJECT_SOURCE_DIR}/music.cpp
		${PROJECT_SOURCE_DIR}/network_fighter_model.cpp
		${PROJECT_SOURCE_DIR}/object_cube.cpp
		${PROJECT_SOURCE_DIR}/particle_effect.cpp
		${PROJECT_SOURCE_DIR}/physics.cpp
		${PROJECT_SOURCE_DIR}/server_networking.cpp
		${PROJECT_SOURCE_DIR}/sound.cpp
		${PROJECT_SOURCE_DIR}/ui_manager.cpp
		${PROJECT_SOURCE_DIR}/text.cpp
		${PROJECT_SOURCE_DIR}/trombone_manager.cpp)

set(SWORD_HEADERS
		${PROJECT_SOURCE_DIR}/bbox.hpp
		${PROJECT_SOURCE_DIR}/cape.hpp
		${PROJECT_SOURCE_DIR}/colour_defines.hpp
		${PROJECT_SOURCE_DIR}/fighter.hpp
		${PROJECT_SOURCE_DIR}/game_state_manager.hpp
		${PROJECT_SOURCE_DIR}/map_tools.hpp
		${PROJECT_SOURCE_DIR}/menu.hpp
		${PROJECT_SOURCE_DIR}/music.hpp
		${PROJECT_SOURCE_DIR}/network_fighter_model.hpp
		${PROJECT_SOURCE_DIR}/object_cube.hpp
		${PROJECT_SOURCE_DIR}/particle_effect.hpp
		${PROJECT_SOURCE_DIR}/physics.hpp
		${PROJECT_SOURCE_DIR}/server_networking.hpp
		${PROJECT_SOURCE_DIR}/sound.hpp
		${PROJECT_SOURCE_DIR}/text.hpp
		${PROJECT_SOURCE_DIR}/ui_manager.hpp
		${PROJECT_SOURCE_DIR}/util.hpp
		${PROJECT_SOURCE_DIR}/trombone_manager.hpp)

set(RENDERER_SOURCES
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/Game/Galaxy/galaxy.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/Game/space_manager.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/camera_effects.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/cl_gl_interop_texture.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/clstate.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/controls.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/engine.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/goo.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/hologram.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/interact_manager.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/light.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/logging.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/network.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/obj_info_struct.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/obj_load.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/object.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/object_context.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/objects_container.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/ocl.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/point_cloud.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/projectile_buffer_gen.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/settings_loader.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/text_handler.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/texture.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/texture_context.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/triangle.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/ui_manager.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/vertex.cpp)

set(RENDERER_HEADERS
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/Game/Galaxy/galaxy.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/Game/space_manager.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/camera_effects.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/cl_gl_interop_texture.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/clstate.h
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/controls.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/engine.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/goo.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/hologram.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/interact_manager.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/light.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/logging.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/network.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/obj_g_descriptor.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/obj_info_struct.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/obj_load.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/object.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/object_context.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/objects_container.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/ocl.h
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/point_cloud.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/proj.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/projectile_buffer_gen.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/settings_loader.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/text_handler.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/texture.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/texture_context.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/triangle.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/ui_manager.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/vec.hpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/vertex.hpp)

set(SERVER_SOURCES
		${PROJECT_SOURCE_DIR}/../sword_server/game_server/game_modes.cpp
		${PROJECT_SOURCE_DIR}/../sword_server/reliability_shared.cpp
		${PROJECT_SOURCE_DIR}/../sword_server/teaminfo_shared.cpp)

set(SERVER_HEADERS
		${PROJECT_SOURCE_DIR}/../sword_server/game_server/game_modes.hpp
		${PROJECT_SOURCE_DIR}/../sword_server/reliability_shared.hpp
		${PROJECT_SOURCE_DIR}/../sword_server/teaminfo_shared.hpp)

set(IMGUI_SOURCES
		${SWORD_EXTERNAL_PROJECTS_PATH}/imgui/imgui.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/imgui/imgui_draw.cpp
		${SWORD_EXTERNAL_PROJECTS_PATH}/imgui-sfml/imgui-SFML.cpp)

set(IMGUI_HEADERS
		${SWORD_EXTERNAL_PROJECTS_PATH}/imgui/imgui.h
		${SWORD_EXTERNAL_PROJECTS_PATH}/imgui-sfml/imgui-SFML.h)

include_directories(
		${PROJECT_SOURCE_DIR}/
		${SWORD_EXTERNAL_PROJECTS_PATH}
		${SWORD_EXTERNAL_PROJECTS_PATH}/imgui
		${SWORD_EXTERNAL_PROJECTS_PATH}/imgui-sfml
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/lib/half/include
		${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/lib/compute/include
)

add_executable(${PROJECT_NAME} ${SWORD_SOURCES} ${SWORD_HEADERS} ${SERVER_SOURCES} ${SERVER_HEADERS} ${RENDERER_SOURCES} ${RENDERER_HEADERS} ${IMGUI_SOURCES} ${IMGUI_HEADERS})

target_link_libraries(
		${PROJECT_NAME}
		${OPENGL_gl_LIBRARY}
		${OpenCL_LIBRARIES}
		${CMAKE_THREAD_LIBS_INIT}
		${SFML_LIBRARIES})

file(COPY ${SWORD_EXTERNAL_PROJECTS_PATH}/OpenCLRenderer/cl2.cl DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
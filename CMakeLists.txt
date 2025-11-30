cmake_minimum_required(VERSION 3.20)
project(lost_sphere LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDKs manuais (você passa *_DIR nas CMake Options do CLion)
find_package(SDL2 CONFIG REQUIRED)          # SDL2-2.32.2
find_package(SDL2_ttf CONFIG REQUIRED)      # SDL2_ttf-2.20.2
find_package(SDL2_image CONFIG REQUIRED)    # SDL2_image-2.8.8
find_package(SDL2_mixer CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/SceneManager.cpp
        src/Assets.cpp
        src/SpriteSheet.cpp
        src/scenes/MenuScene.cpp
        src/scenes/OverworldScene.cpp
        src/scenes/BattleScene.cpp
        src/scenes/SelectSetScene.cpp
        src/scenes/BossIntroScene.cpp
        src/TileMap.cpp
        src/SoundManager.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE include)

target_link_libraries(${PROJECT_NAME} PRIVATE
        SDL2::SDL2
        SDL2::SDL2main
        SDL2_ttf::SDL2_ttf
        SDL2_image::SDL2_image
        SDL2_mixer::SDL2_mixer
)

# Copiar DLLs pro lado do executável (Windows)
if (WIN32)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "$<TARGET_FILE:SDL2::SDL2>"         "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "$<TARGET_FILE:SDL2_ttf::SDL2_ttf>" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "$<TARGET_FILE:SDL2_image::SDL2_image>"   "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "$<TARGET_FILE:SDL2_mixer::SDL2_mixer>"   "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
  )
endif()

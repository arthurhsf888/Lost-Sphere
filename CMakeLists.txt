cmake_minimum_required(VERSION 3.20)
project(lost_sphere LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Você vai passar SDL2_DIR e SDL2_ttf_DIR nas CMake Options do CLion
find_package(SDL2 CONFIG REQUIRED)          # Ex.: C:\Program Files (x86)\SDKs\SDL2-2.32.2\cmake
find_package(SDL2_ttf CONFIG REQUIRED)      # Ex.: C:\Program Files (x86)\SDKs\SDL2_ttf-2.20.2\cmake
# Se/quando a gente usar imagens ou áudio, habilitamos depois:
# find_package(SDL2_image CONFIG REQUIRED)  # Ex.: C:\Program Files (x86)\SDKs\SDL2_image-2.8.8\cmake
# find_package(SDL2_mixer CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/SceneManager.cpp
        src/scenes/MenuScene.cpp
        src/scenes/OverworldScene.cpp
        src/scenes/BattleScene.cpp
        src/scenes/SelectSetScene.cpp
        include/Balance.h
        include/Boss.h
        include/BossFuria.h
        src/scenes/BossIntroScene.h
        src/scenes/BossIntroScene.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE include)

# Link usando os targets exportados pelos SDKs oficiais
target_link_libraries(${PROJECT_NAME} PRIVATE
        SDL2::SDL2
        SDL2::SDL2main
        SDL2_ttf::SDL2_ttf
        # SDL2::SDL2_image   # habilite quando formos usar
        # SDL2_mixer::SDL2_mixer
)

# (Windows) Copiar as DLLs ao lado do executável após o build (facilita rodar no CLion)
if (WIN32)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "$<TARGET_FILE:SDL2::SDL2>"
          "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "$<TARGET_FILE:SDL2_ttf::SDL2_ttf>"
          "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
          # Descomente se habilitar Image/Mixer:
          # COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:SDL2::SDL2_image>" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
          # COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:SDL2_mixer::SDL2_mixer>" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
  )
endif()

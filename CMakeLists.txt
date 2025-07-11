cmake_minimum_required(VERSION 3.10)
project(unisinos-computer-graphics)

# Define o padrão do C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Ativa o FetchContent
include(FetchContent)

# Configura o FetchContent para baixar e compilar GLFW automaticamente
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4  # Define a versão desejada da GLFW
)

# Configura o FetchContent para baixar a GLM automaticamente
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG master  # Define a versão desejada da GLM
)

# Faz o download e compila as bibliotecas
FetchContent_MakeAvailable(glfw glm)

# Adiciona as pastas de cabeçalhos
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/common)
include_directories(${CMAKE_SOURCE_DIR}/include/glad)
include_directories(${glm_SOURCE_DIR})

# Lista de exemplos/exercícios podem ser colocados aqui também
set(EXERCISES
    Hello3D
    Suzannes
    Cube3D
    SuzanneTexture
    SuzannePhong
    SuzanneCamera
    UniverseCurves
    FinalProject
)

add_compile_options(-Wno-pragmas)

# Define as bibliotecas
find_package(OpenGL REQUIRED)
set(OPENGL_LIBS ${OPENGL_gl_LIBRARY})

# Caminho esperado para a GLAD
set(GLAD_C_FILE "${CMAKE_SOURCE_DIR}/common/glad.c")

# Verifica se os arquivos da GLAD estão no lugar
if (NOT EXISTS ${GLAD_C_FILE})
    message(FATAL_ERROR "Arquivo glad.c não encontrado! Baixe a GLAD manualmente em https://glad.dav1d.de/ e coloque glad.h em include/glad/ e glad.c em common/")
endif()

add_library(stb_image STATIC src/stb_image.cpp)
target_include_directories(stb_image PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Cria os executáveis
foreach(EXERCISE ${EXERCISES})
    set(SOURCES src/${EXERCISE}.cpp ${GLAD_C_FILE})

    # Se for Suzannes, adiciona LoadSimpleOBJ.cpp também
    if(EXERCISE STREQUAL "Suzannes")
        list(APPEND SOURCES module2/vivencial/LoadSimpleOBJ.cpp)
    endif()

    if(EXERCISE STREQUAL "SuzanneCamera" OR EXERCISE STREQUAL "UniverseCurves" OR EXERCISE STREQUAL "FinalProject")
        list(APPEND SOURCES include/Camera/camera.cpp)
    endif()

    if(EXERCISE STREQUAL "FinalProject")
        list(APPEND SOURCES include/Bezier/bezier.cpp)
    endif()
    
    add_executable(${EXERCISE} ${SOURCES})

    target_include_directories(${EXERCISE} PRIVATE ${CMAKE_SOURCE_DIR}/include/glad ${glm_SOURCE_DIR} ${stb_image_SOURCE_DIR})
    target_link_libraries(${EXERCISE} glfw ${OPENGL_LIBS} stb_image)
endforeach()

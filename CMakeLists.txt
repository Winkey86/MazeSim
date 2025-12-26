cmake_minimum_required(VERSION 3.20)
project(MazeLab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MSVC: speed up builds & get good diagnostics
if (MSVC)
  add_compile_options(/W4 /permissive-)
endif()

add_executable(MazeLab
  src/main.cpp

  src/core/RNG.cpp
  src/core/Maze.cpp

  src/generators/RecursiveBacktrackerGenerator.cpp
  src/generators/PrimGenerator.cpp
  src/generators/CellularAutomataGenerator.cpp

  src/pathfinding/BFSPathfinder.cpp
  src/pathfinding/AStarPathfinder.cpp

  src/agents/BFSAgent.cpp
  src/agents/AStarAgent.cpp
  src/agents/RightHandAgent.cpp
  src/agents/FrontierExplorerAgent.cpp
  src/agents/ManualAgent.cpp

  src/sim/Simulation.cpp
  src/sim/Leaderboard.cpp

  src/ui/Widgets.cpp
  src/ui/Renderer.cpp
  src/ui/App.cpp
)

target_include_directories(MazeLab PRIVATE src)

# SFML (vcpkg, config package)
find_package(SFML 3 CONFIG REQUIRED COMPONENTS Graphics Window System)

# Link targets (SFML 3 uses SFML::*, some setups expose sfml-*)
if (TARGET SFML::Graphics)
  target_link_libraries(MazeLab PRIVATE SFML::Graphics SFML::Window SFML::System)
elseif (TARGET sfml-graphics)
  target_link_libraries(MazeLab PRIVATE sfml-graphics sfml-window sfml-system)
else()
  message(FATAL_ERROR "SFML targets not found. Check SFML installation / vcpkg toolchain.")
endif()

# On Windows, ensure console is shown (handy for logs). You can change to WIN32 if you want no console.

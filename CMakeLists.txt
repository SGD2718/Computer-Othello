cmake_minimum_required(VERSION 3.26)
project(Othello)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# include libraries
find_package(Torch REQUIRED)
include_directories(${CMAKE_SOURCE_DIR}/lib/QCustomPlot)
include_directories(${TORCH_INCLUDE_DIRS})

# Find the Qt package
set(CMAKE_PREFIX_PATH "/opt/homebrew/Cellar/qt/6.6.1/")
find_package(Qt6 COMPONENTS Core Gui Widgets PrintSupport REQUIRED)

# add executables
add_executable(
        Othello
        src/main.cpp
        src/Const.h
        src/Engine/Masks.h
        src/Game/Board.cpp
        src/Game/Board.h
        src/Engine/Engine.h
        src/Engine/Engine.cpp
        src/Engine/Search/SearchStructs.h
        src/Engine/Search/MidSearch.cpp
        src/Engine/Search/EndSearch.cpp
        src/Engine/Search/MidSearchNWS.cpp
        src/Engine/Evaluation/EvalBuilder.cpp
        src/Engine/Evaluation/EvalBuilder.h
        src/Bit.h
        lib/QCustomPlot/qcustomplot.cpp
        lib/QCustomPlot/qcustomplot.h
        src/Engine/Search/TranspositionTable.cpp
        src/Engine/Search/TranspositionTable.h
        src/Game/Game.cpp
        src/Game/Game.h
        src/Game/Move.h
        src/Game/Move.cpp
        src/Engine/Evaluation/TernaryIndices.h
        src/Engine/Evaluation/StaticEvaluations.h
        src/Util.cpp
        src/Util.h
        src/GUI/BoardWidget.cpp
        src/GUI/BoardWidget.h
        src/GUI/CellWidget.cpp
        src/GUI/CellWidget.h
        src/GUI/QtInclude.h
        src/GUI/ScoreWidget.cpp
        src/GUI/ScoreWidget.h
        src/GUI/ScoreDiscWidget.cpp
        src/GUI/ScoreDiscWidget.h
        src/GUI/AiConfigWidget.cpp
        src/GUI/AiConfigWidget.h
        src/GUI/OthelloGui.cpp
        src/GUI/OthelloGui.h
        src/GUI/SidePanelWidget.cpp
        src/GUI/SidePanelWidget.h
        src/GUI/EvaluationWidget.cpp
        src/GUI/EvaluationWidget.h
        src/GUI/AiWorker.cpp
        src/GUI/AiWorker.h
        src/Engine/Search/Etc.cpp
        src/Engine/Search/MoveOrdering.cpp
        src/Engine/Search/EndSearchLastN.cpp
        src/Engine/Evaluation/Evaluation.h
        src/Engine/Evaluation/Evaluation.cpp
        src/Engine/Evaluation/MirrorFeature.cpp
        src/Engine/Evaluation/EvalBuilderDebug.cpp
        src/Init.h
        src/Engine/Evaluation/LinearModel.h
        src/Engine/Search/ProbCut.cpp
)

# Link Qt6Core to your application
target_link_libraries(Othello PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::PrintSupport ${TORCH_LIBRARIES})

# add compile options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Enable optimizations that promote inlining and vectorization
    add_compile_options(-O3 -finline-functions -finline-small-functions -findirect-inlining -ftree-vectorize -march=native)
    add_compile_options(src/Engine/Search/ProbCut.cpp -03 -ffast-math -finline-functions -finline-small-functions -findirect-inlining -ftree-vectorize -march=native)
endif()
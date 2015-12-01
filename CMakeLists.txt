 #
 # @brief JUDAH - Jacob is equipped with a text-based user interface
 #
 # @file CMakeLists.txt
 # @author  Norbert Bátfai <nbatfai@gmail.com>
 # @version 0.0.1
 #
 # @section LICENSE
 #
 # Copyright (C) 2015 Norbert Bátfai, batfai.norbert@inf.unideb.hu
 #
 # This program is free software: you can redistribute it and/or modify
 # it under the terms of the GNU General Public License as published by
 # the Free Software Foundation, either version 3 of the License, or
 # (at your option) any later version.
 #
 # This program is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 # GNU General Public License for more details.
 #
 # You should have received a copy of the GNU General Public License
 # along with this program.  If not, see <http://www.gnu.org/licenses/>.
 #
 # @section DESCRIPTION
 #
 # JACOB, https://github.com/nbatfai/jacob
 #
 # "The son of Isaac is Jacob." The project called Jacob is an experiment 
 # to replace Isaac's (GUI based) visual imagination with a character console.
 #
 # ISAAC, https://github.com/nbatfai/isaac
 #
 # "The son of Samu is Isaac." The project called Isaac is a case study 
 # of using deep Q learning with neural networks for predicting the next 
 # sentence of a conversation.
 # 
 # SAMU, https://github.com/nbatfai/samu
 #
 # The main purpose of this project is to allow the evaluation and 
 # verification of the results of the paper entitled "A disembodied 
 # developmental robotic agent called Samu Bátfai". It is our hope 
 # that Samu will be the ancestor of developmental robotics chatter 
 # bots that will be able to chat in natural language like humans do.
 # 
 
cmake_minimum_required(VERSION 2.8.5)
project(SAMU)

## remote
# add_definitions(-DSUPER_OR_REMOTE_COMP)
#
# #set(CUDA_LAYERS ON)
# set(CUDA_LAYERS OFF)
#
# #set(DISP_CURSES ON)
# set(DISP_CURSES OFF)

# TUI
 #add_definitions(-DSUPER_OR_REMOTE_COMP)
 
 #set(CUDA_LAYERS ON)
 set(CUDA_LAYERS OFF)

 set(DISP_CURSES ON)
 #set(DISP_CURSES OFF)

# Amminadab
add_definitions(-DLZW_TREE)
add_definitions(-DNETCHAT)

  
# Ram
add_definitions(-DINCREMENTAL_CACHE)
add_definitions(-DCELL_AUTOMATA)
#add_definitions(-DFOUR_TIMES)
#add_definitions(-DDRAW_WNUM)
#add_definitions(-DPLACE_VALUE)
#add_definitions(-DFEELINGS)
#add_definitions(-DPRINTING_CHARBYCHAR)
add_definitions(-DSARSA)

# Hezron 
# add_definitions(-DPYRAMID_VI)
# add_definitions(-DJUSTIFY_VI)

# Judah 
add_definitions(-DTRIPLET_CACHE)

# Jacob 
add_definitions(-DCHARACTER_CONSOLE)

# uncomment to use Q lookup table (for test purposes only)
#add_definitions(-DQ_LOOKUP_TABLE)

add_definitions(-DQNN_DEBUG_BREL)
add_definitions(-DQNN_DEBUG)
#add_definitions(-DRND_DEBUG)
#add_definitions(-DNN_DEBUG)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -O3 -lm")

if(${CUDA_LAYERS})
  add_definitions(-DCUDA_PRCPS)
  find_package(CUDA REQUIRED)
  set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-O3 -std=c++11 
  -gencode arch=compute_30,code=sm_30)
endif()

# Judah
if(${DISP_CURSES})
  add_definitions(-DDISP_CURSES)
  find_package(Curses REQUIRED)
  include_directories(${CURSES_INCLUDE_DIR})
endif()

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

find_package( Boost COMPONENTS system date_time REQUIRED )  

include_directories( ${Boost_INCLUDE_DIRS} ) 
link_directories( ${Boost_LIBRARY_DIRS} )  

include(FindPNGwriter.cmake)

if(PNGwriter_FOUND)
    include_directories(SYSTEM ${PNGwriter_INCLUDE_DIRS})
    add_definitions(${PNGwriter_DEFINITIONS})
    set(LIBS ${LIBS} ${PNGwriter_LIBRARIES})
endif(PNGwriter_FOUND)

include(FindLinkGrammar.cmake)

include_directories(${LINK_GRAMMAR_INCLUDE_DIRS})

find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/doxydoc/htm/index.html)
    add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxydoc/htm/index.html
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM)
endif(DOXYGEN_FOUND)

add_custom_target(
    srs
    COMMAND make
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc
)

if (CUDA_FOUND)
  cuda_compile(CUDASRCS qlc.cu)
  cuda_add_executable(samu ${CUDASRCS} disp.hpp net.hpp nlp.hpp nlp.cpp qlc.h ql.hpp samu.hpp samu.cpp main.cpp )
else()
                   add_executable(samu disp.hpp net.hpp nlp.hpp nlp.cpp       ql.hpp samu.hpp samu.cpp main.cpp  )
endif()

target_link_libraries(samu ${LINK_GRAMMAR_LIBRARIES} ${PNGwriter_LIBRARIES} ${Boost_LIBRARIES} ${CURSES_LIBRARIES})

install(TARGETS samu DESTINATION bin)

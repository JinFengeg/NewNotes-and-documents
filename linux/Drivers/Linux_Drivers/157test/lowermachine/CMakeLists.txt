


# Builds the minimal Tensorflow Lite example.

cmake_minimum_required(VERSION 3.16)
project(Myemg C CXX)

set(TENSORFLOW_SOURCE_DIR "" CACHE PATH
  "Directory that contains the TensorFlow project"
)
if(NOT TENSORFLOW_SOURCE_DIR)
  get_filename_component(TENSORFLOW_SOURCE_DIR
    "${CMAKE_CURRENT_LIST_DIR}/../../../../"
    ABSOLUTE
  )
endif()

add_subdirectory(
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite"
  "${CMAKE_CURRENT_BINARY_DIR}/tensorflow-lite"
  EXCLUDE_FROM_ALL
)

set(CMAKE_CXX_STANDARD 11)
add_executable(Myemg
  main.cc
  global.cc
  Sem.cc
  shm.cc
  main_data.cc
  data_WIN.cc
  main_dsp.cc
  BP_DATA.cc
  main_tcp.cc
  tcp_TCPClass.cc
  data_ADC_main.cc
  ADC_OL.cc
  filt_ADC.cc
  data_IMU_main.cc
  IMU_data.cc
  dsp_ACT.cc
  dsp_RCG_main.cc
  dsp_RCGclass.cc
  BPNet.cc
  data_FOOT_main.cc
  class_Dense.cc
  CAN_class.cc
  main_can.cc
)
target_link_libraries(Myemg
  tensorflow-lite
  pthread
  wiringPi
)

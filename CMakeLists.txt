CMAKE_MINIMUM_REQUIRED(VERSION 3.4.0)

PROJECT(MedicalImageProcessing)

CMAKE_POLICY(SET CMP0020 NEW)
CMAKE_POLICY(SET CMP0028 NEW)
CMAKE_POLICY(SET CMP0053 NEW)

SET(CMAKE_INCLUDE_CURRENT_DIR ON)

#---�����Զ�����moc�ļ�,����ȱ��---------
#--������仰֮����Բ���дQT5_WRAP_CPP(WRAP_SRCS_H ${HEARD_FILES})
#
SET(CMAKE_AUTOMOC ON)

#------------------------------------------
FIND_PACKAGE(Qt5Widgets)
FIND_PACKAGE(Qt5Core)
FIND_PACKAGE(Qt5Gui)
FIND_PACKAGE(OpenCV REQUIRED)
FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})


#--------���ҵ�ǰ�ļ��е�����Դ��----------

FILE(GLOB SRC_FILES "./*.cpp")
FILE(GLOB HEAD_FILES "./*.h")
FILE(GLOB UI_FILES "./*.ui")

#--------����ui�ļ���ͷ�ļ�----------------
QT5_WRAP_UI(WRAP_FILES ${UI_FILES})
#QT5_WRAP_CPP(WRAP_SRCS_H ${HEARD_FILES})

#--------������Դ�ļ�----------------------
#SET(RCC_FILES rcc.qrc)

#--------���������ļ�-----------------------
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

ADD_EXECUTABLE(${PROJECT_NAME}
               ${SRC_FILES}
               ${HEAD_FILES}
               ${UI_FILES}
               ${WRAP_SRCS_H}
               ${WRAP_FILES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
                     ${OpenCV_LIBS}
					 ${VTK_LIBRARIES}    
                      Qt5::Widgets
                      Qt5::Core 
                      Qt5::Gui) 
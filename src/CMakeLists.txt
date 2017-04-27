# 增加opencv的依赖
FIND_PACKAGE( OpenCV REQUIRED )

# 
##ADD_EXECUTABLE( learning1 tut1.cpp )
##TARGET_LINK_LIBRARIES( learning1 ${OpenCV_LIBS})

##ADD_EXECUTABLE( camera_calib camera_calib.cpp )
##TARGET_LINK_LIBRARIES( camera_calib ${OpenCV_LIBS}

##ADD_EXECUTABLE( filter filter.cpp )
##TARGET_LINK_LIBRARIES(filter ${OpenCV_LIBS} 
##)


ADD_EXECUTABLE( opencv_gst opencv_gstream.cpp )
TARGET_LINK_LIBRARIES(opencv_gst  ${OpenCV_LIBS} )


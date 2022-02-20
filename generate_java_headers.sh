#!/bin/sh

#  generate_java_headers.sh
#  
#
#  Created by Steven Meyer on 10/19/21.
#  

javah -v -d src/main/native/source -classpath bin/main com.brazedblue.intervox.view3D.OpenGLJNI

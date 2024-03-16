#!/bin/sh

#  Intervox.sh
#  IntervoxLib
#
#  Created by Steven Meyer on 10/2/22.
#  

export set CLASSPATH="classes/java/main:packages/vecmath-1.5.2.jar:packages/rxtx-2.1.7.jar:packages/jama-1.0.3.jar"
java -Djava.library.path="lib"  com.brazedblue.intervox.main.NSAppletFrameDev

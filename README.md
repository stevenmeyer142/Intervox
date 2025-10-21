# Intervox Image Guided Surgery Application

This App was developed as a proof of concept around the year 2000. It displayed Dicom images in a java application. It was written before OpenGL support in Java. It's OpenGL rendering was done offscreen, and it ran only on older Mac versions. To upgrade the app, the OpenGL code was converted to Vulkan.

The app interfaced with a localizer to display DICOM images during surgery. It was used non-commercially in over 500 surgeries.

# Prerequisites

## VCPKG

`VCPKG_ROOT` must be set.

For Windows:
```
$env:VCPKG_ROOT = "C:\path\to\vcpkg"
$env:PATH = "$env:VCPKG_ROOT;$env:PATH"
```

For Linux:
```
export VCPKG_ROOT="path\to\vcpkg"
```

### Build dependencies
autoconfig, pkg-config, automake, libtool

## CMAKE

## TODO

Getting an error on Mac "Could not create Vulkan instance : ERROR_INCOMPATIBLE_DRIVER"
Clean up CMAKEXXFlags vs targetcompileroption
I had to comment out lines 44-46 in mvk_vulkan.h



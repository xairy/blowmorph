﻿Blowmorph
==========

[![Build Status](https://travis-ci.org/bmteam/blowmorph.svg?branch=master)](https://travis-ci.org/bmteam/blowmorph)
[![Build Status](https://ci.appveyor.com/api/projects/status/lecflmqj7erg90ps?svg=true)](https://ci.appveyor.com/project/xairy/blowmorph)


A simple crossplatform 2D multiplayer shooter.

More infromation is available in the Wiki.

A demo can be found in the Releases section.

## Running

To run the client run:
```
./launcher.sh
```

To run the server configure blowmorph/data/server.cfg and run:
```
./server.sh
```

To run the master server configure blowmorph/data/master-server.cfg and run:
```
./master-server.sh
```

## Building

### Ubuntu

1. Install SFML-2.2 from http://www.sfml-dev.org/. See the instructions below.

2. Install Box2D-2.3.0 from http://box2d.org/. See the instructions below.

3. Install ENet:
    ```
    sudo apt-get install libenet-dev libenet2a
    ```

4. Install libjsoncpp:
    ```
    sudo apt-get install libjsoncpp-dev
    ```

5. Install wxWidgets for Python:
    ```
    sudo apt-get install python-wxtools
    ```

6. Install premake4:
    ```
    sudo apt-get install premake4
    ```
    
    If the packet is not available, get the binary from http://industriousone.com/premake/download.

7. Generate project files:
    ```
    premake4 gmake
    ```

8. On x32 system:
    ```
    cd build; make config=release32
    ```
On x64 system:
    ```
    cd build; make config=release64
    ```

### Windows

1. Install Python 2.7.9.

2. Download third-party.zip and extract in the blowmorph folder.

3. Download premake5.exe from http://industriousone.com/premake/download.

4. Run `premake5.exe vs2013`.

5. Build build\blowmorph.sln.
    
## Installing dependencies

### SFML-2.2

1. Install cmake:
    ```
    sudo apt-get install cmake
    ```

2. Install the required dependencies:
    ```
    sudo apt-get install libpthread-stubs0-dev libgl1-mesa-dev \
    libxrandr-dev libfreetype6-dev libglew-dev libjpeg8-dev \
    libsndfile1-dev libopenal-dev libudev-dev
    ```

3. Download SFML-2.2-sources.zip from http://www.sfml-dev.org/download/sfml/2.2/ and extract.

4. Build SFML-2.2:
    ```
    cd SFML-2.2
    cmake .
    make
    sudo make install
    ```

### Box2D-2.3.0

1. Install cmake:
    ```
    sudo apt-get install cmake
    ```

2. Install the required dependencies:
    ```
    sudo apt-get install freeglut3-dev libxi-dev
    ```

3. Download Box2D-2.3.0 source from https://box2d.googlecode.com/files/Box2D_v2.3.0.7z and extract.

4. Build Box2D-2.3.0:
    ```
    cd Box2D/
    cmake -DBOX2D_INSTALL=ON -DBOX2D_BUILD_SHARED=ON .
    make
    sudo make install
    ```

### wxFormBuilder

1. Add PPA for wxWidgets v3.0:
    ```
    sudo add-apt-repository -y ppa:wxformbuilder/wxwidgets
    sudo apt-get update
    ```

2. Install prerequisites:
    ```
    sudo apt-get install libwxgtk3.0-0 libwxgtk-media3.0-0
    ```

3. Add PPA for wxFormBuilder:
    ```
    sudo add-apt-repository -y ppa:wxformbuilder/release
    sudo apt-get update
    ```

4. Install wxFormBuilder:
    ```
    sudo apt-get install wxformbuilder
    ```

### Tiled

1. Add Tiled PPA:
    ```
    sudo add-apt-repository ppa:mapeditor.org/tiled
    sudo apt-get update
    ```

2. Install Tiled:
    ```
    sudo apt-get install tiled
    ```

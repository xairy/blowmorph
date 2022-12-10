Blowmorph
==========

[![Build Status](https://travis-ci.org/bmteam/blowmorph.svg?branch=master)](https://travis-ci.org/bmteam/blowmorph)
[![Build Status](https://ci.appveyor.com/api/projects/status/lecflmqj7erg90ps?svg=true)](https://ci.appveyor.com/project/xairy/blowmorph)

A simple crossplatform 2D multiplayer shooter.

Written in 2013-2015 as a part of a university course.

Use [Docker](https://github.com/xairy/blowmorph/tree/master/docker) for building and running on Linux.

A very old Linux and Windows demo can be found in the Releases section.

![](/files/screenshot.png)

## Client controls

- WASD: Move
- E: Open door
- Left mouse click: Shoot rocket
- Right mouse click: Shoot morph slime
- TAB: See score table
- ESC: Exit

## Running

Optionally, configure `data/master-server.json` and run the master server:

```
./master-server.sh
```

Configure `data/server.json` and run the server:

``` bash
./server.sh
```

Run the client launcher:

``` bash
./launcher.sh
```

Or configure `data/client.json` and run the clieny manually:

``` bash
./client.sh
```

## Building

### Ubuntu

1. Install SFML-2.2 from http://www.sfml-dev.org/. See the instructions below.

2. Install Box2D-2.3.0 from http://box2d.org/. See the instructions below.

3. Install ENet:

    ``` bash
    sudo apt-get install libenet-dev libenet2a
    ```

4. Install libjsoncpp:

    ``` bash
    sudo apt-get install libjsoncpp-dev
    ```

5. Install wxWidgets for Python:

    ``` bash
    sudo apt-get install python-wxtools
    ```

6. Install premake4:

    ``` bash
    sudo apt-get install premake4
    ```

    If the packet is not available, get the binary from http://industriousone.com/premake/download.

7. Generate project files:

    ``` bash
    premake4 gmake
    ```

8. On x64 system:

    ``` bash
    cd build && make config=release32
    ```

    On x32 system:

    ``` bash
    cd build && make config=release64
    ```

### Windows

1. Install Python 2.7.9.

2. Download [third-party.zip](https://github.com/xairy/blowmorph/releases/download/v0.1-alpha/third-party.zip) and extract in the blowmorph directory.

3. Download premake5.exe from http://industriousone.com/premake/download.

4. Run `premake5.exe vs2013`.

5. Build `build\blowmorph.sln`.

## Installing dependencies

### SFML-2.2

1. Install cmake:

    ``` bash
    sudo apt-get install cmake
    ```

2. Install the required dependencies:

    ``` bash
    sudo apt-get install libpthread-stubs0-dev libgl1-mesa-dev \
    libxrandr-dev libfreetype6-dev libglew-dev libjpeg8-dev \
    libsndfile1-dev libopenal-dev libudev-dev
    ```

3. Download SFML-2.2-sources.zip from http://www.sfml-dev.org/download/sfml/2.2/ and extract.

4. Build SFML-2.2:

    ``` bash
    cd SFML-2.2
    cmake .
    make
    sudo make install
    ```

### Box2D-2.3.0

1. Install cmake:

    ``` bash
    sudo apt-get install cmake
    ```

2. Install the required dependencies:

    ``` bash
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

    ``` bash
    sudo add-apt-repository ppa:wxformbuilder/wxwidgets
    sudo apt-get update
    ```

2. Install prerequisites:

    ``` bash
    sudo apt-get install libwxgtk3.0-0 libwxgtk-media3.0-0
    ```

3. Add PPA for wxFormBuilder:

    ``` bash
    sudo add-apt-repository ppa:wxformbuilder/release
    sudo apt-get update
    ```

4. Install wxFormBuilder:

    ``` bash
    sudo apt-get install wxformbuilder
    ```

### Tiled

1. Add Tiled PPA:

    ``` bash
    sudo add-apt-repository ppa:mapeditor.org/tiled
    sudo apt-get update
    ```

2. Install Tiled:

    ``` bash
    sudo apt-get install tiled
    ```

## Style checker

To set up pre-commit hook for style checking add the following to `.git/hooks/pre-commit`:

```
#!/bin/sh

./scripts/checkstyle.py
RESULT=$?
[ $RESULT -ne 0 ] && exit 1
exit 0
```

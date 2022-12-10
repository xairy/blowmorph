FROM ubuntu:trusty

RUN \
	apt-get update &&						\
	apt-get -y upgrade &&						\
	apt-get install -y build-essential cmake &&			\
	apt-get install -y wget unzip p7zip-full git vim &&		\
	apt-get install -y x11-apps mesa-utils libgl1-mesa-glx

ENV HOME /home/user
RUN useradd --create-home --home-dir $HOME user \
	&& chown -R user:user $HOME \
	&& usermod -a -G audio,video user

WORKDIR /blowmorph/

RUN \
	apt-get install -y libpthread-stubs0-dev libgl1-mesa-dev	\
		libxrandr-dev libfreetype6-dev libglew-dev libjpeg8-dev	\
		libsndfile1-dev libopenal-dev libudev-dev &&		\
	wget --no-check-certificate https://www.sfml-dev.org/files/SFML-2.2-sources.zip && \
	unzip SFML-2.2-sources.zip &&					\
	cd SFML-2.2/ && mkdir build && cd build/ &&			\
	cmake .. && make && make install

RUN \
	apt-get install -y freeglut3-dev libxi-dev &&			\
	wget --no-check-certificate https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/box2d/Box2D_v2.3.0.7z && \
	7za x Box2D_v2.3.0.7z && cd Box2D_v2.3.0/Box2D/ &&		\
	mkdir build && cd build/ &&					\
	cmake -DBOX2D_INSTALL=ON -DBOX2D_BUILD_SHARED=ON .. &&		\
	make && make install

# Based on https://github.com/gklingler/docker3d.
# Comment out for non-Nvidia setup:
ADD NVIDIA-driver.run /tmp/NVIDIA-driver.run
RUN sh /tmp/NVIDIA-driver.run -a -N --ui=none --no-kernel-module
RUN rm /tmp/NVIDIA-driver.run

# Uncomment for non-Nvidia setup:
# RUN	apt-get install -y xserver-xorg-video-all

RUN \
	apt-get install -y libenet-dev libenet2a &&			\
	apt-get install -y libjsoncpp-dev &&				\
	apt-get install -y premake4 &&					\
	apt-get install -y python python-requests python-wxtools &&	\
	apt-get install -y tiled

WORKDIR $HOME
USER user

RUN \
	git clone --depth=1 https://github.com/xairy/blowmorph.git &&	\
	cd blowmorph/ && premake4 gmake &&				\
	cd build/ && make config=release64

# https://github.com/Sentdex/Carla-RL/issues/6.
ENV QT_X11_NO_MITSHM=1

CMD ["bash"]

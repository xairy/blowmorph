Docker container for running Blowmorph
======================================

For Nvidia-based systems, get [Nvidia drivers](https://stackoverflow.com/a/44187181):

``` bash
version="$(glxinfo | grep "OpenGL version string" | rev | cut -d" " -f1 | rev)"
wget http://us.download.nvidia.com/XFree86/Linux-x86_64/"$version"/NVIDIA-Linux-x86_64-"$version".run
mv NVIDIA-Linux-x86_64-"$version".run NVIDIA-driver.run
```

For non-Nvidia-based systems, edit `Dockerfile` according to the comments inside.

Build the container with `./build.sh`.

Run the container with `./run.sh`.

In the container, start server and client:

```
user@docker:~/blowmorph$ ./server.sh &
[1] 24
user@docker:~/blowmorph$ ./client.sh
```

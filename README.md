# PROJECT_NAME_PLACEHOLDER

OpenGL template project with wrappers around the basic OpenGL objects.

This contains two examples: Application and ApplicationMinimal.

Application has lighting, while ApplicationMinimal is just simple scene with no lighting.

## Building and Running

### Windows (Visual Studio)

The easiest way to build is to use [vcpkg](https://vcpkg.io/en/index.html) and install dependencies through this:

```bash
vcpkg install sfml
vcpkg install imgui
vcpkg install glm
vcpkg integrate install
```

Then open the Visual Studio project file to build and run.

### Linux

#### Pre-requisites

Install Vcpkg and other required packages using your distribution's package manager:

```sh
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh

# These are required to build some packages
sudo apt install cmake make autoconf libtool pkg-config

# The following are required for SFML
sudo apt install libx11-dev xorg-dev freeglut3-dev libudev-dev
```

Ensure paths are set correctly:

```sh
export VCPKG_ROOT=/path/to/vcpkg
export PATH=$VCPKG_ROOT:$PATH
```

RECOMMENDED: Add the above lines to your `.bashrc` or `.zshrc` file:

```sh
echo 'export VCPKG_ROOT=/path/to/vcpkg' >> ~/.bashrc
echo 'export PATH=$VCPKG_ROOT:$PATH' >> ~/.bashrc
```

#### Build and Run

To build, at the root of the project:

```sh
vcpkg install # First time only
sh scripts/build.sh
```

To run, at the root of the project:

```sh
sh scripts/run.sh
```

To build and run in release mode, simply add the `release` suffix:

```sh
sh scripts/build.sh release
sh scripts/run.sh release
```

## Credits

--

Grass texture: https://opengameart.org/content/tileable-grass-textures-set-1

--

Box texture from LearnOpenGL: https://learnopengl.com/Lighting/Lighting-maps

--

Skybox texture from ThinMatrix tutorial, also from here: https://opengameart.org/content/miramar-skybox

https://www.dropbox.com/sh/phslacd8v9i17wb/AABui_-C-yhKvZ1H2wb3NykIa?dl=0
https://www.youtube.com/watch?v=_Ix5oN8eC1E

--

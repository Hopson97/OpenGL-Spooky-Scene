# dsa_practice

description here

### Windows (Visual Studio)

The easiest way to build is to use [vcpkg](https://vcpkg.io/en/index.html) and install libraries through this.

```bash
vcpkg install sfml
vcpkg install imgui
vcpkg install assimp
vcpkg install glm
vcpkg integrate install
```

Create a new visual studio C++ empty project, git clone the files, and copy them into the project directory.

Select the "Show All Files" options in Solution Explorer, and right-click on the src/ and deps/ directory, and choose the "include in project options"

Go into the project properies and under `C/C++ > General`, add the deps/ directory as an additional include directy.

Finally, under `Linker > Input`, add OpenGL32.lib as an additional dependancy.

Under "Project -> Properties -> C/C++ -> Preprocessor -> Preprocessor Definitions" add `_CRT_SECURE_NO_WARNINGS`z

### Linux

Requires conan.

```sh
python3 -m pip install conan
```

To build, at the root of the project:

```sh
sh scripts/build.sh install
```

The install argument is only needed for the first time compilation as this is what grabs the libraries from Conan.

So after the first time, you can simply run:

```
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

[]: https://www.sfml-dev.org/download/sfml/2.5.1/

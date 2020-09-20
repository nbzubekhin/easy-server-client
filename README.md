# easy-tcp-udp-server-client

This project contains a simple TCP / UDP server and client implementation.

# Build the project

When building `EASY-TCP-UDP-SERVER-CLIENT` as a standalone project on Unix-like systems with Cmake/GNU Make, the typical workflow is:

1. Get the source code and change to it

```
git clone https://github.com/nbzubekhin/easy-tcp-udp-server-client.git

cd easy-tcp-udp-server-client/
```

2. Run CMake to configure the build tree
```
cmake -H. -Bbuild -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
```
or
```
cmake -H. -Bbuild -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug
```

3. Afterwards, generated files can be used to compile the project
```
cmake --build build
```

4. Test the build software (optional)

The version used for building the tests is Google Mock 1.6.0. We need unzip the installation zip file (used gmock-1.6.0.zip), perhaps in our home directory. We will also need to build Google Test, which is nested within Google Mock.
```
cd $GMOCK_HOME/gtest
mkdir mybuild
cd mybuild
cmake ..
make
```

Building and running tests
```
export GMOCK_HOME=$GMOCK_HOME_DIRECTORY/gmock-1.6.0
cmake -H. -Bbuild -G "Unix Makefiles" -DBUILD_UNIT_TESTS=ON
cmake --build build
```

5. To build the samples, do the following steps
```
cmake -H. -Bbuild -G "Unix Makefiles" -DBUILD_SAMPLES=ON
cmake --build build
```

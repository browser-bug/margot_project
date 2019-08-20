# README
This repository contains the mARGOt framework core, version 2

## Summary of the framework

The mARGOt framework core provides to an application the ability to dynamically adapt, in order to face changes in the execution environment or in the application requirements. The framework exploits the information gathered during a Design Space Exploration in order to guide the selection of the most suitable configuration of software knobs according to application requirements.

The application requirements are expressed as a constrained multi-objective optimization problem. The framework enables the application to change, at runtime, the optimization problem (e.g. redefining the objective funcion or adding a new constraint ). Moreover, the framework enables the user to change also the application knowledge at runtime, providing support for an online Design Space Exploration.

### Code organization

The repository is organized as follow:
```
├── margot_heel/
|   ├── margot_heel_cli/  -> Command Line Interface, it might:
|   |                            - generate the glue code to ease the integration (MARGOT_HEEL)
|   |                            - genarate a gnuplot script that plot a list of Operating Points
|   |                            - filter the Pareto-dominated Operating Points
|   |                            - additional minor features to manage the list of Operating Points
|   |                            - plot execution traces of the tuned applications
|   ├── margot_heel_if/   -> An high-level interface template, to ease the integration process
|                            It uses margot_heel_cli to generate the glue code
|
├── framework/
|   ├── cmake/            -> CMake files used to find dependecy libraries (for some monitors)
│   ├── config/           -> Framework configuration files to find mARGOt in the target application:
|   |                         - the template used to generate FinMARGOT.cmake for cmake
|   |                         - the template of mARGOt ".pc" file for pkg-config
|   |
|   ├── doc/              -> Configuration files to generate Doxygen documentation
|   ├── include/          -> Header files of the mARGOt autotuner and AGORA application handler
|   ├── src/              -> Source files of the mARGOt autotuner and AGORA application handler
|   ├── test/             -> Test suite files, exploiting cxxtest
|
├── agora/
|   ├── plugins/          -> Collection of tools to build the application knowledge from observations
|   |                         - crs: leverage nonparametric regression splines
|   |                         - average: compute the average between observed configurations
|   |                         - hth: advanced plugin to model a metrics
|   |
│   ├── src/              -> The AGORA remote application handler source files
|
├── extra/
|   ├── benchmark/        -> A benchmark application to evaluate the framework overheads
```

### Compiling instructions
The building system is based on CMake. Assuming you are in the path path/to/repository/root, the default procedure is as follow:
~~~
:::bash
$ mkdir build
$ cd build
$ cmake -DCMAKE_INSTALL_PREFIX:PATH=<path> ..
$ make
$ make install
~~~
In order to build the framework it is required gcc > 4.8.1.
The framework itself is complaint to the C++11 standard, making it generic; however, several monitors parse the /proc metafiles, assuming a unix-like environment


#### Building option
The default configuration builds and installs the framework as a static library, using as much monitors as possible.
However, it is possible to change this behavior using the CMake configuration options as follows:

| Option name              |  Values [default]  | Description                                                 |
|--------------------------|--------------------|-------------------------------------------------------------|
| LIB_STATIC               |  [ON],  OFF        | Build a static library (otherwise it is shared)             |
| WITH_DOC                 |   ON , [OFF]       | Generate the Doxygen documentation                          |
| WITH_TEST                |   ON , [OFF]       | Build the cxxtest application, to test the framework        |
| USE_COLLECTOR_MONITOR    |   ON , [OFF]       | Include the wrapper monitor for Examon (by ETHz)            |
| USE_PAPI_MONITOR         |   ON , [OFF]       | Include the monitor of Perf events (using PAPI interface)   |
| USE_TEMPERATURE_MONITOR  |   ON , [OFF]       | Include the temperature monitor (requires lm_sensors)       |
| WITH_AGORA               |   ON , [OFF]       | Enable the AGORA application handler (see NOTE below)       |
| WITH_BENCHMARK           |   ON , [OFF]       | Build a benchmark to evaluate the overheads                 |
| WITH_CASSANDRA           |   ON , [OFF]       | Enable the cassandra storage back-end                       |

NOTE: if you are interested on using the AGORA application handler to perform an online Design Space Exploration, the module adds several dependencies:
 - The C/C++ Cassandra driver ( http://datastax.github.io/cpp-driver/ )
 - The C/C++ MQTT client implementation ( https://www.eclipse.org/paho/ )

The build system is able to automatically download and compile those libraries if they are not available, which is the preferred option.
However, to compile them, we have the following requirements:
 - The uv library ( https://github.com/libuv/libuv ), on fedora is available in the repositories
 - The OpenSSL libraries, usually they are available on the repositories
 - The pthread library, usually already installed in most distributions


### Contribution guidelines
Clone the repository and send pull requests, any contribution is welcome.

### Who do I talk to?
Contact: davide [dot] gadioli [at] polimi [dot] it

Organization: Politecnico di Milano, Italy


### Acknowledgment
This work has been supported by European Commission under the grant 671623 FET-HPC-ANTAREX (AutoTuning and Adaptivity appRoach for Energy efficient eXascale HPC systems)


# Description

The solution can be found at GitHub [https://github.com/ciechowoj/devices-demo](https://github.com/ciechowoj/devices-demo). The solution consists of three executables: device, server and test. The test executable runs unit tests and beside that isn't interesting. The device binary is a device simulator, it can be run with the following command line:
`device <host> <device_id> <period> [<enable-logging>]`,
for example:
`device localhost 42 0.01 false`

The device program repeatedly (every `period` seconds) sends JSON-encoded messages to the `host` ip using UDP. Here is an example of such a message:
```
{
  "device_id": 42,
  "serial_id": 77,
  "timestamp": 28551359047656,
  "measurement": 1585990364
}
```

The device id is the `device_id` parameter from the command line, the messages have incremental serials ids, the timestamp is number of nanoseconds obtained using monotonic system clock (using unix'es `clock_gettime(...)` routine). The measurement is a random number generated with `rand()`. The device simulator is used to test the server program. Multiple `device`s can be run simultaneously (however they cannot have the same device id).

The server program listens at the port 1911 for the messages sent over UDP. The port number is compiled-in, but can be changed in CMakeLists.txt. The server program counts the number of received messages and every second prints a JSON formatted message about the status to the standard output, for example:

```
{ "received": 0, "lost": 0 }
{ "received": 0, "lost": 0 }
{ "received": 0, "lost": 0 }
{ "received": 4, "lost": 0 }
{ "received": 103, "lost": 0 }
{ "received": 201, "lost": 0 }
{ "received": 299, "lost": 0 }
[...]
```

The `received` variable is the number of messages received so far, the `lost` denotes the number of messages that had been lost. I'm assuming that the system doesn't require 100% acquisition of the measurements. The loss is caused by an inherent unreliability of the UDP protocol. The `timestamp` and `serial_id` fields of the message are used to deal with duplicates (which may happen with UDP) and to guarantee basic ordering of the measurements (the out-of-order datagrams are considered lost).

# How to build the solution
## The tools required to build the solution:
The build was tested on clean Linux Mint 19.1 installation, with below, minimal set of tools:
```
apt-get install git
apt-get install g++-8
apt-get install python3-pip
pip3 install cmake
pip3 install setuptools # for conan
pip3 install conan
```
## The steps to build the solution:
After above tools are installed, below are the commands required to build the solution. Conan will install two dependencies ASIO library of async programming and Haste-UnitTest for unit tests. Then the makefiles are generated using cmake and finally the project is built using make.

```
conan remote add ciechowoj https://api.bintray.com/conan/ciechowoj/Haste
git clone https://github.com/ciechowoj/devices-demo.git
cd devices-demo
./build.sh
```

After the build the binaries can be found in the `./build/bin` directory.

# How to see the demo in action
From one terminal window run:
```
./build/bin/server
```
open two (or more) terminals and in every of them run (it's important for the devices to have different ids):
```
./build/bin/device localhost 1001 0.01 false # terminal 1
./build/bin/device localhost 1002 0.01 false # terminal 2
./build/bin/device localhost 1003 0.01 false # terminal 3
./build/bin/device localhost 1004 0.01 false # terminal 4
[...]
```

observe the messages logged by the server.

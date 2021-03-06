
# What is QuickHub?

QuickHub is a toolkit or SDK that provides basic features of a typical IoT or cloud solution in a generalized way. The goal is to reduce the development effort for own cloud, IoT and HMI scenarios to an absolute minimum by modularizing and abstracting typical features.

The QuickHub framework consists of three core components:

  - The server application
  - The client library for implementing user interfaces with Qt QML  ([Link to the repository](https://github.com/quickhub-project/quickhub-qmlclientmodule))
  - The embedded framework for rapid firmware development of QuickHub devices. Based on the Espressif SDKs for the ESP8266 and ESP32 respectively. 

## What are the advantages of QuickHub? 

QuickHub offers a variety of pre-built components that can be used to implement custom IoT, cloud or HMI solutions. 

### Qt on the client side

QML, as Qt's declarative UI description language, is well suited for implementing chic and contemporary graphical interfaces thanks to high-performance OpenGL rendering and simple syntax. Interfaces based on Qt QML can be executed on all relevant desktop and mobile platforms. This makes it possible to support all platforms from Windows, Linux, OSX to Android and iOS with a single code base. Since the generic QuickHub interface is already fully implemented in the form of a middleware in a QML module, one only needs to worry about the individual visualization of the data.  Everything else, from connection establishment to ready-made QAbstractItemModels is fully implemented by the QuickHub ClientLibrary for direct use from QML. ([Link to the repository](https://github.com/quickhub-project/quickhub-qmlclientmodule))

### Qt on the server side 

Since QuickHub is built entirely on C++ and Qt on the client and server side, the framework is extremely economical in terms of memory and CPU usage. It runs on almost all relevant platforms thanks to Qt's cross-platform compatibility, including weak embedded computers like the Raspberry Pi. This makes QuickHub particularly suitable for edge devices and IoT gateways, such as those often used in the SmartHome sector. 

Due to its resource-efficient implementation, QuickHub can also be encapsulated very well in containers, which also makes it very suitable for off-site hosting of individual and security-critical SaaS solutions.


### Extensible via plugins 

The modular architecture of QuickHub provides different plugin interfaces that allow to extend the functionality of the server in terms of provided services. The QuickHub Core provides a complete infrastructure for this purpose. The goal of QuickHub is to provide uniform interfaces for data structures in order to separate the concrete, server-side representation of the data from the access protocols used by the clients.

If a srver-side plugin registers a service, it can be accessed through the WebSocket interface provided by the QuickHub core. Services can provide data in form of resources and functionality. Resources can be implemented either in the structure of key/value stores or lists. Functionality, on the other hand, can be provided by synchronous or asynchronous RPCs. 

By restricting the data to a few predefined structures, the data can be perfectly abstracted from the interface used.  In addition to the WebSocket interface provided by QuickHub by default, thanks to the modular architecture, plugins can be registered that allow access via alternative protocols and interfaces.  For example, there is also a ready-made QuickHub plugin that also provides the registered services via a REST-like HTTP API. The same would be conceivable for MQTT, Modbus, CAN or any other interfaces.???The standard WebSocket-based interface provided by QuickHub is part of the core. It works seamlessly with the corresponding client library and ensures that changes to the resources of a service are synchronized with all clients in real time. The big advantage is that the developer of a service does not have to deal with protocol details and infrastructure development on either the server or client side, as QuickHub already provides this in full.  The infrastructure provided by QuickHub also includes user management with rights management, which can be used to ensure that only authorized users can access certain resources or services.

## Common use cases

### QuickHub as HMI middleware

QuickHub can also be used very well as middleware for HMIs. In this case, all system functions are registered with QuickHub through plugins, which can then be accessed via the WebSocket abstraction. 
This approach has many advantages: 

- The HMI can run as an independent process alongside the application logic without having to worry about complex and error-prone thread synchronization. 
- The HMI can also run directly as a standalone app on a tablet or remote device. This means that the code for the HMI can be almost completely reused when developing a phone or tablet app.
- Due to the use of QuickHub you are forced to strictly follow the rules of an MVC pattern in your software architecture design.


### QuickHub as (IoT-) Device-Hub 

In addition to services, QuickHub also manages devices and client-side access to them. But what is a device? In the QuickHub context, a device consists of a unique DeviceID and a set of properties and RPCs. A Device does not necessarily have to be a physical hardware device. It can also be a virtual device that exists only on the software side.

Devices log on to the server either via the websocket interface or via external plug-ins and can then be registered in the system by an authorized user. This gives the device a freely definable, unique address with which it can be addressed from this point on.

Thanks to the IDFix SDK, QuickHub devices can be easily implemented on the Espressif ESP8266 and ESP32 microcontrollers. The complete interface between server and microcontroller, as well as all relevant functions for provisioning are already fully provided by the QuickHub SDK for Espressif (IDFix). *IDFix is currently not open source but may be released soon.*

The QuickHub Device Gateway offers the following functionalities:

- Connection management (detect and correctly handle disconnections).
- Device Management with Authentication (Devices must be registered to a unique address to be accessible via QuickHub).
- Property Shadowing (If a device is offline, property changes are cached until the device logs in again)
- Property Confirmation (The server expects a confirmation for each property change. On the client side, this makes it immediately apparent whether a property change has been processed by a device or not.
- Modularization via generalized interfaces, whereby the device concept can be extended as desired with the help of plug-ins.
- Alternative interfaces extensible via plug-ins (e.g. CAN, Modbus, MQTT)


# How do I get it to work?

## Docker

If you just need a plain, ready to use QuickHub instance without any third part plugins you can use the official docker image. It's provided for arm and x86 so it also works on the raspberry pi.

```docker
docker run -p 4711:4711 -v /local/quickhub/files:/quickhub/ frime/quickhub
```

## Build it yourself

This is straight forward:

```
git clone https://github.com/quickhub-project/quickhub.git
cd quickhub
git submodule init
git submodule update
mkdir build
cd build
qmake ..
make -j4
```




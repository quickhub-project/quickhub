# What is QuickHub?

QuickHub is a toolkit or SDK that provides basic features of a typical IoT or cloud solution in a generalized way. The goal is to reduce the development effort for own cloud and IoT scenarios to an absolute minimum by modularizing and abstracting typical features.

The QuickHub framework consists of three core components:

  - The server application
  - The client library for implementing user interfaces with Qt QML
  - The embedded framework for rapid firmware development of QuickHub devices. Based on the Espressif SDKs for the ESP8266 and ESP32 respectively. 

# What are the advantages of QuickHub? 

QuickHub offers a variety of pre-built components that can be used to implement custom IoT and cloud solutions. 

## Qt on the server side 

Since QuickHub is built entirely on C++ and Qt on the client and server side, the framework is extremely economical in terms of memory and CPU usage. It runs on almost all relevant platforms thanks to Qt's cross-platform compatibility, including weak embedded small computers like the Raspberry Pi. This makes QuickHub particularly suitable for edge devices and IoT gateways, such as those often used in the SmartHome sector. 

Due to its resource-efficient implementation, QuickHub can also be encapsulated very well in containers, which also makes it very suitable for off-site hosting of individual and security-critical SaaS solutions.


## Qt on the client side

QML, as Qt's declarative UI description language, is well suited for implementing chic and contemporary graphical interfaces thanks to high-performance OpenGL rendering and simple syntax. Interfaces based on Qt QML can be executed on all relevant desktop and mobile platforms. This makes it possible to support all platforms from Windows, Linux, OSX to Android and iOS with a single code base. Since the generic QuickHub interface is already fully implemented in the form of a middleware in a QML module, one only needs to worry about the individual visualization of the data.  Everything else, from connection establishment to ready-made QAbstractItemModels is fully implemented by the QuickHub ClientLibrary for direct use from QML.


# The server 

The server application is fully implemented in C++ using the cross-platform framework Qt. The modular architecture of QuickHub provides different plugin interfaces that allow to extend the functionality of the server in terms of provided services. The QuickHub Core provides a complete infrastructure for this purpose. The goal of QuickHub is to provide uniform interfaces for data structures in order to separate the concrete, server-side representation of the data from the access protocols used by the clients.  

## Services and interfaces

 If a server-side plugin registers a service, it can be accessed through the WebSocket interface provided by the QuickHub core. Services always consist of a set of resources and services. Resources can be implemented either in the structure of key/value stores or lists. Services, on the other hand, represent synchronous or asynchronous RPCs. 

By restricting the data to a few predefined structures, the data can be perfectly abstracted from the interface used.  In addition to the WebSocket interface provided by QuickHub by default, thanks to the modular architecture, plugins can be registered that allow access via alternative protocols and interfaces.  For example, there is also a ready-made QuickHub plugin that also provides the registered services via a REST-like HTTP API. The same would be conceivable for MQTT, Modbus, CAN or any other interfaces.   The standard WebSocket-based interface provided by QuickHub is part of the core. It works seamlessly with the corresponding client library and ensures that changes to the resources of a service are synchronized with all clients in real time. The big advantage is that the developer of a service does not have to deal with protocol details and infrastructure development on either the server or client side, as QuickHub already provides this in full.  The infrastructure provided by QuickHub also includes user management with rights management, which can be used to ensure that only authorized users can access certain resources or services.


## Devices 

In addition to services, QuickHub also manages devices and client-side access to them. (IoT) devices also log on to the server via the websocket interface and can then be registered in the system by an authorized user. This assigns the device a freely definable, unique address with which it can be addressed from that moment on.

Thanks to the IDFix SDK, QuickHub devices can be easily implemented on the Espressif ESP8266 and ESP32 microcontrollers. The complete interface between server and microcontroller, as well as all relevant functions for provisioning are already fully provided by the QuickHub SDK for Espressif (IDFix).

# Architecture of the QuickHub Core 

Here is a simplified representation of the most important classes to illustrate the individual abstraction layers.

![image](https://user-images.githubusercontent.com/51061627/118320428-87eac380-b4fc-11eb-9362-5158a462bbef.png)



## Dependencies

The QuickHub Core has dependencies to the following Qt modules: QtCore, QtWebsockets and QtConcurrent. There is also a dependency on the QHPluginSystem, which organizes the loading and management of the individual plugins. Also the QHPluginSystem has no other dependencies besides the QtCore.


## Separation between protocol and data layer

A special feature of QuickHub are the separation layers on several levels between the entities (blue) and the protocol or transport layer (green). Thanks to unified interfaces, different protocols can thus be implemented that provide access to the data in different ways. Due to the abstraction by factories on both sides, the functionality can be extended by plug-ins with the help of own implementations. The basic infrastructure, which takes care of object management and connection management, for example, is thus also implicitly used for custom implementations.

## The protocol layer 

The endpoint for WebSocket connections is a separate subdir project within the QuickHob Core and could theoretically be considered a separate plugin. However, we decided to treat the WebSocket protocol as a fixed part of the core, since the corresponding client and device implementations depend on this module, making the QuickHub core a self-sufficient plugin.


## Virtual Connections and IResourceHandler

Each WebSocket connection as wrapper around the Qt class `QWebSocket` (class `Connection` derived from `IConnectable`) allows to open any number of virtual "sub-connections" via a concrete connection. These `ISocket` instances connect e.g. concrete model instances of a client with the corresponding server endpoints.

In concrete terms, this means that as soon as a SynchronizedListModel is instantiated in QML on the client side, an ISocket object is dropped from the connection on the server side, which is then assigned (attached) to the corresponding endpoint, an `IResourceHandler`. For each entity there is a separate `IResourceHandler` implementation, which implements the corresponding JSON protocol.  If 5 clients are connected to a synchronized list via a `SynchronizedListHandler` this means that 5 `ISocket` instances are attached to the resource handler. If one of the clients now wants to modify the list via a JSON message, the ResourceHandler accepts this message and tries to execute the change. If the modification is successful, the ResourceHandler sends this modification via `deployAll()` to all ISocket instances (also to the client that caused the modification). This ensures that all clients are on the same state at all times.  


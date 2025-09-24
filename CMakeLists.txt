cmake_minimum_required(VERSION 3.20)
project(QHCore)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core WebSockets Concurrent Gui)

# Set target name
set(TARGET_NAME QHCore)

# Base source files from QHCore.pri
set(CORE_SOURCES
    src/Server/Authentication/AuthentificationService.cpp
    src/Server/Authentication/Controller.cpp
    src/Server/Authentication/IIdentitiy.cpp
    src/Server/Authentication/User.cpp
    src/Connection/VirtualConnection.cpp
    src/Connection/Connection.cpp
    src/Server/Devices/DevicePermissionManager.cpp
    src/Server/Devices/DeviceService.cpp
    src/Server/Devices/DeviceUpdateLogic.cpp
    src/Server/Resources/ListResource/ListResource.cpp
    src/Server/Resources/ListResource/QObjectListResource.cpp
    src/Server/Resources/ObjectResource/QObjectResource.cpp
    src/Server/Resources/ResourceManager/ResourceManager.cpp
    src/Server/Resources/ObjectResource/ObjectResource.cpp
    src/Server/Resources/ResourceManager/IResource.cpp
    src/Server/Devices/DeviceManager.cpp
    src/Server/Resources/ListResource/ListResourceFactory.cpp
    src/Server/Resources/ObjectResource/ObjectResourceFactory.cpp
    src/Server/Devices/IDevice.cpp
    src/Server/Devices/DeviceHandle.cpp
    src/Server/Devices/DeviceProperty.cpp
    src/Server/Settings/SettingsManager.cpp
    src/Server/Settings/SettingsResource.cpp
    src/Storage/FileSystemPaths.cpp
    src/Storage/ListResourceFileSystemStorage.cpp
    src/Storage/ObjectResourceFilesystemStorage.cpp
    src/Server/Services/ServiceManager.cpp
    src/Storage/ListResourceTemporaryStorage.cpp
    src/Server/Authentication/IUser.cpp
    src/Server/Authentication/DefaultAuthenticator.cpp
    src/Storage/FileSystemLoader.cpp
    # GUI-related sources (always included for Qt6)
    src/Server/Resources/ImageResource/ImageResource.cpp
    src/Server/Resources/ImageResource/ImageResourceFactory.cpp
    src/Storage/ImageResourceFilesystemStorage.cpp
)

# SocketApi sources
set(SOCKETAPI_SOURCES
    src/SocketApi/Devices/SocketDeviceHandler.cpp
    src/SocketApi/ResourceHandler/SynchronizedList/SynchronizedListHandler.cpp
    src/SocketApi/ResourceHandler/SynchronizedList/SynchronizedListHandlerFactory.cpp
    src/SocketApi/ResourceHandler/SynchronizedObject/SynchronizedObjectHandler.cpp
    src/SocketApi/ResourceHandler/SynchronizedObject/SynchronizedObjectHandlerFactory.cpp
    src/SocketApi/DataHandler/Lists/IList.cpp
    src/SocketApi/DataHandler/Lists/ListWrapper/DeviceListWrapper.cpp
    src/SocketApi/SocketCore/SocketResourceManager.cpp
    src/SocketApi/SocketCore/IResourceHandlerFactory.cpp
    src/SocketApi/SocketServer.cpp
    src/SocketApi/Session/SessionHandler.cpp
    src/SocketApi/SocketCore/IResourceHandler.cpp
    src/SocketApi/DataHandler/Lists/ListHandler.cpp
    src/SocketApi/DataHandler/Lists/ListHandlerFactory.cpp
    src/SocketApi/DataHandler/Lists/ListWrapper/UserListWrapper.cpp
    src/SocketApi/DataHandler/Lists/ListWrapper/DeviceHandleListWrapper.cpp
    src/SocketApi/Devices/DeviceHandleHandler.cpp
    src/SocketApi/Devices/DeviceHandleHandlerFactory.cpp
    src/SocketApi/Devices/SocketDevice.cpp
    src/SocketApi/Services/ServiceRequestHandler.cpp
    # GUI-related SocketApi sources
    src/SocketApi/ResourceHandler/ImageCollection/ImageCollectionHandler.cpp
    src/SocketApi/ResourceHandler/ImageCollection/ImageCollectionHandlerFactory.cpp
)

# Base header files
set(CORE_HEADERS
    src/Server/Authentication/AuthentificationService.h
    src/Server/Authentication/Controller.h
    src/Server/Authentication/IIdentitiy.h
    src/Server/Authentication/User.h
    src/Connection/VirtualConnection.h
    src/Connection/Connection.h
    src/Server/Devices/DevicePermissionManager.h
    src/Server/Devices/DeviceService.h
    src/Server/Resources/ListResource/ListResourceFactory.h
    src/Server/Devices/DeviceUpdateLogic.h
    src/Server/Devices/IDevicePermissionController.h
    src/Server/Resources/ListResource/ListResource.h
    src/Server/Resources/ListResource/QObjectListResource.h
    src/Server/Resources/ObjectResource/QObjectResource.h
    src/Server/Resources/ResourceManager/IResource.h
    src/Server/Resources/ResourceManager/ResourceManager.h
    src/Server/Resources/ObjectResource/ObjectResource.h
    src/Server/Devices/DeviceManager.h
    src/Server/Resources/ResourceManager/IResourceFactory.h
    src/Server/Resources/ObjectResource/ObjectResourceFactory.h
    src/Server/Devices/IDevice.h
    src/Server/Defines/ErrDef.h
    src/Server/Devices/DeviceHandle.h
    src/Connection/ISocket.h
    src/Connection/IConnectable.h
    src/Server/Settings/SettingsManager.h
    src/Server/Settings/SettingsResource.h
    src/Storage/FileSystemPaths.h
    src/Server/Devices/DeviceProperty.h
    src/Server/Resources/ListResource/IListResourceStorage.h
    src/Storage/ListResourceFileSystemStorage.h
    src/Server/Resources/ListResource/IListResourceStorageFactory.h
    src/Server/Resources/ObjectResource/IObjectResourceStorage.h
    src/Server/Resources/ObjectResource/IObjectResourceStorageFactory.h
    src/Storage/ObjectResourceFilesystemStorage.h
    src/Server/Logging/Logger.h
    src/Server/Services/ServiceManager.h
    src/Server/Services/IService.h
    src/Storage/ListResourceTemporaryStorage.h
    src/Server/Authentication/IAuthenticator.h
    src/Server/Authentication/IUser.h
    src/Server/Authentication/DefaultAuthenticator.h
    src/Storage/FileSystemLoader.h
    # GUI-related headers
    src/Server/Resources/ImageResource/ImageResource.h
    src/Server/Resources/ImageResource/ImageResourceFactory.h
    src/Server/Resources/ImageResource/IImageResourceStorage.h
    src/Server/Resources/ImageResource/IImageResourceStorageFactory.h
    src/Storage/ImageResourceFilesystemStorage.h
)

# Plugin-specific sources and headers
set(PLUGIN_SOURCES
    src/QHCorePlugin.cpp
)

set(PLUGIN_HEADERS
    src/qhcore_global.h
    src/QHCorePlugin.h
)

# All sources and headers
set(ALL_SOURCES ${CORE_SOURCES} ${SOCKETAPI_SOURCES} ${PLUGIN_SOURCES})
set(ALL_HEADERS ${CORE_HEADERS} ${PLUGIN_HEADERS})

# Create shared library
add_library(${TARGET_NAME} SHARED ${ALL_SOURCES} ${ALL_HEADERS})

# Link Qt6 libraries
target_link_libraries(${TARGET_NAME} 
    Qt6::Core 
    Qt6::WebSockets 
    Qt6::Concurrent 
    Qt6::Gui
    QHPluginSystem
)

# Include directories
target_include_directories(${TARGET_NAME} PUBLIC 
    src
    src/SocketApi
    ../quickhub-pluginsystem/src
)

# Set output directory to plugins
set_target_properties(${TARGET_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugins
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugins
)

# Add compile definitions
target_compile_definitions(${TARGET_NAME} PRIVATE 
    COREPLUGIN_LIBRARY
    SKIP_EMPTY_PARTS=Qt::SkipEmptyParts
)
cmake_minimum_required(VERSION 3.15)
project(launcher Java)
find_package(Java 1.7 REQUIRED COMPONENTS Development)

include(UseJava)
set(CMAKE_JAVA_JAR_ENTRY_POINT org.cleanlauncher.EntryPoint)
set(CMAKE_JAVA_COMPILE_FLAGS -target 8 -source 8)

set(SRC
    org/cleanlauncher/EntryPoint.java
    org/cleanlauncher/launcher/Launcher.java
    org/cleanlauncher/launcher/impl/AbstractLauncher.java
    org/cleanlauncher/launcher/impl/StandardLauncher.java
    org/cleanlauncher/exception/ParameterNotFoundException.java
    org/cleanlauncher/exception/ParseException.java
    org/cleanlauncher/utils/Parameters.java
    org/cleanlauncher/utils/ReflectionUtils.java
    org/cleanlauncher/utils/logging/Level.java
    org/cleanlauncher/utils/logging/Log.java
    org/cleanlauncher/legacy/LegacyProxy.java
)

set(LEGACY_SRC
    legacy/org/cleanlauncher/legacy/LegacyFrame.java
    legacy/org/cleanlauncher/legacy/LegacyLauncher.java
    legacy/org/cleanlauncher/legacy/fix/online/Handler.java
    legacy/org/cleanlauncher/legacy/fix/online/OnlineFixes.java
    legacy/org/cleanlauncher/legacy/fix/online/OnlineModeFix.java
    legacy/org/cleanlauncher/legacy/fix/online/SkinFix.java
    legacy/org/cleanlauncher/legacy/utils/Base64.java
    legacy/org/cleanlauncher/legacy/utils/api/MojangApi.java
    legacy/org/cleanlauncher/legacy/utils/api/Texture.java
    legacy/org/cleanlauncher/legacy/utils/json/JsonParseException.java
    legacy/org/cleanlauncher/legacy/utils/json/JsonParser.java
    legacy/org/cleanlauncher/legacy/utils/url/ByteArrayUrlConnection.java
    legacy/org/cleanlauncher/legacy/utils/url/UrlUtils.java
    legacy/net/minecraft/Launcher.java
    legacy/org/cleanlauncher/legacy/LegacyProxy.java
)

add_jar(NewLaunch ${SRC})
add_jar(NewLaunchLegacy ${LEGACY_SRC} INCLUDE_JARS NewLaunch)
install_jar(NewLaunch "${JARS_DEST_DIR}")
install_jar(NewLaunchLegacy "${JARS_DEST_DIR}")

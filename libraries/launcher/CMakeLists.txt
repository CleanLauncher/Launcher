cmake_minimum_required(VERSION 3.15)
project(launcher Java)
find_package(Java 1.7 REQUIRED COMPONENTS Development)

include(UseJava)
set(CMAKE_JAVA_JAR_ENTRY_POINT org.launcher.EntryPoint)
set(CMAKE_JAVA_COMPILE_FLAGS -target 8 -source 8)

set(SRC
    org/launcher/EntryPoint.java
    org/launcher/launcher/Launcher.java
    org/launcher/launcher/impl/AbstractLauncher.java
    org/launcher/launcher/impl/StandardLauncher.java
    org/launcher/exception/ParameterNotFoundException.java
    org/launcher/exception/ParseException.java
    org/launcher/utils/Parameters.java
    org/launcher/utils/ReflectionUtils.java
    org/launcher/utils/logging/Level.java
    org/launcher/utils/logging/Log.java
    org/launcher/legacy/LegacyProxy.java
)

set(LEGACY_SRC
    legacy/org/launcher/legacy/LegacyFrame.java
    legacy/org/launcher/legacy/LegacyLauncher.java
    legacy/org/launcher/legacy/fix/online/Handler.java
    legacy/org/launcher/legacy/fix/online/OnlineFixes.java
    legacy/org/launcher/legacy/fix/online/OnlineModeFix.java
    legacy/org/launcher/legacy/fix/online/SkinFix.java
    legacy/org/launcher/legacy/utils/Base64.java
    legacy/org/launcher/legacy/utils/api/MojangApi.java
    legacy/org/launcher/legacy/utils/api/Texture.java
    legacy/org/launcher/legacy/utils/json/JsonParseException.java
    legacy/org/launcher/legacy/utils/json/JsonParser.java
    legacy/org/launcher/legacy/utils/url/ByteArrayUrlConnection.java
    legacy/org/launcher/legacy/utils/url/UrlUtils.java
    legacy/net/minecraft/Launcher.java
    legacy/org/launcher/legacy/LegacyProxy.java
)

add_jar(NewLaunch ${SRC})
add_jar(NewLaunchLegacy ${LEGACY_SRC} INCLUDE_JARS NewLaunch)
install_jar(NewLaunch "${JARS_DEST_DIR}")
install_jar(NewLaunchLegacy "${JARS_DEST_DIR}")

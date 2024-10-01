cmake_minimum_required(VERSION 3.13)
project(sound_meter)

include_directories(src)

add_executable(sound_meter
	src/main.c
	src/process.h
	src/process.c
	src/config.h
	src/config.c
	src/filter.h
	src/filter.c
	src/in_out.h
	src/in_out.c
	src/sbuffer.h
	src/sbuffer.c
	src/mqtt.h
	src/mqtt.c
	src/server.c
	)

	find_package(PkgConfig REQUIRED)
	pkg_check_modules(deps REQUIRED IMPORTED_TARGET jansson libwave alsa glib-2.0 paho-mqtt3c)
	target_link_libraries(sound_meter PkgConfig::deps m)
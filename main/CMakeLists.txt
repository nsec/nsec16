# SPDX-FileCopyrightText: 2024 NorthSec
#
# SPDX-License-Identifier: MIT

set(CONFIG_NSEC_BUILD_CONFERENCE $ENV{CONFIG_NSEC_BUILD_CONFERENCE} CACHE INTERNAL "")
set(CONFIG_NSEC_BUILD_CTF $ENV{CONFIG_NSEC_BUILD_CTF} CACHE INTERNAL "")
set(CONFIG_NSEC_BUILD_ADDON $ENV{CONFIG_NSEC_BUILD_ADDON} CACHE INTERNAL "")

if (CONFIG_NSEC_BUILD_CTF)
    idf_component_register(SRCS "ctf-main.cpp" INCLUDE_DIRS ".")
elseif (CONFIG_NSEC_BUILD_ADDON)
    idf_component_register(SRCS "addon-main.cpp" INCLUDE_DIRS ".")
elseif (CONFIG_NSEC_BUILD_CONFERENCE)
    idf_component_register(SRCS "conf-main.cpp" INCLUDE_DIRS ".")
else()
    message(FATAL_ERROR "Invalid firmware configuration.")
endif()

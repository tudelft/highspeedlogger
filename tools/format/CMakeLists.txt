cmake_minimum_required (VERSION 2.6) 

add_definitions( -D_CRT_SECURE_NO_WARNINGS )
# add_definitions( -Wwrite-strings )

add_executable ( highspeedlog_formatter format.cpp )


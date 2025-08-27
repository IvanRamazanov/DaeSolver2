include(../tests.pri)

INCLUDEPATH += ../../src/MathPack/ tests/build
HEADERS += mem_api.h
SOURCES += test_symbolic_algebra.cpp \
			mem_api.cpp \
			$$files("../../src/MathPack/StringGraph/*.cpp") \
			$$files("../../src/MathPack/Workspace/*.cpp") \
			$$files("../../src/MathPack/MathFunctions/*.cpp") \
			../../src/MathPack/MathFunction.cpp \
			../../src/MathPack/Parser.cpp
			
CONFIG += debug

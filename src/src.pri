include($$QT_SYSTEMKIT_BUILD_TREE/config.pri)

contains(build_unit_tests, yes) {
    CONFIG += debug

    QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
    QMAKE_CLEAN    += *.gcno *.gcda
    QMAKE_LIBS     += -lgcov
}

!CONFIG(debug, debug|release) {
    DEFINES += QT_NO_DEBUG_OUTPUT QT_NO_WARNING_OUTPUT
}

CONFIG += create_pc create_prl

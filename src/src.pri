DEBUG=$$DEBUG
TESTS=$$TESTS

isEqual(DEBUG, 1) | isEqual(TESTS, 1) {
    CONFIG += debug
}

isEqual(TESTS, 1) {
    QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
    QMAKE_CLEAN    += *.gcno *.gcda
    QMAKE_LIBS     += -lgcov
}

CONFIG(release, debug|release) {
    DEFINES += QT_NO_DEBUG_OUTPUT QT_NO_WARNING_OUTPUT
}

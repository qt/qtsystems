TEMPLATE = subdirs
CONFIG += ordered

module_qtsystems_src.subdir = src
module_qtsystems_src.target = module-qtsystems-src

module_qtsystems_tests.subdir = tests
module_qtsystems_tests.target = module-qtsystems-tests
module_qtsystems_tests.depends = module_qtsystems_src
!contains(QT_BUILD_PARTS,tests) {
    module_qtsystems_tests.CONFIG = no_default_target no_default_install
}

SUBDIRS += module_qtsystems_src \
           module_qtsystems_tests

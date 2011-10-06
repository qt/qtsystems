TEMPLATE = subdirs
CONFIG += ordered

module_qtsystems_src.subdir = src
module_qtsystems_src.target = module-qtsystems-src

module_qtsystems_tests.subdir = tests
module_qtsystems_tests.target = module-qtsystems-tests
module_qtsystems_tests.depends = module_qtsystems_src
!contains(QT_BUILD_PARTS,tests):!with-tests {
    module_qtsystems_tests.CONFIG = no_default_target no_default_install
}

module_qtsystems_examples.subdir = examples
module_qtsystems_examples.target = module-qtsystems-exampels
module_qtsystems_examples.depends = module_qtsystems_src
!contains(QT_BUILD_PARTS,examples):!with-examples {
    module_qtsystems_examples.CONFIG = no_default_target no_default_install
    warning("No examples being used")
}

SUBDIRS += module_qtsystems_src \
           module_qtsystems_tests \
           module_qtsystems_examples

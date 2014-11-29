TEMPLATE = subdirs
SUBDIRS += auto

linux-*: !simulator: {
  SUBDIRS += manual/sysinfo-tester
}

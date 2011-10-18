TEMPLATE = subdirs
wince*|symbian:{
  SUBDIRS = test
} else {
  SUBDIRS = test stdinprocess
}

!symbian{!blackberry*:SUBDIRS += largefile}

CONFIG += parallel_test

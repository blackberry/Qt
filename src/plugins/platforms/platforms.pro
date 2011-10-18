TEMPLATE = subdirs

SUBDIRS += minimal
SUBDIRS += blackberry

contains(QT_CONFIG, wayland) {
    SUBDIRS += wayland
}


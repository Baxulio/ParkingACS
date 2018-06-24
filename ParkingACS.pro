TEMPLATE = subdirs

SUBDIRS += \
    Core \
    AccessController \

AccessController.depends = Core

TEMPLATE = subdirs

SUBDIRS += \
    Core \
    AccessController \
    Client \

AccessController.depends = Core
Client.depends = Core

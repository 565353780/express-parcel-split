TEMPLATE = subdirs
SUBDIRS += \
    Q3D \
    test

test.depends = Q3D

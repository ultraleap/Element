# This file is only used on Linux to find the system libusb. We don't
# really support Linux but Samir uses it and one day we might.
#
# For Windows and OSX we build libusb in-tree.

find_path(RapidJSON_DIR "../dependencies/rapidjson")

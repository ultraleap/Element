import os
import shutil
import glob
import sys


def copy_file(source, destination):
    for filename in glob.glob(source):
        if os.path.isfile(filename):
            shutil.copy(filename, destination)
            print("Moved {} to {}".format(filename, destination))
        else:
            print("File {} does not exist.".format(filename))


def create_zip_file(input_path, filename):
    if os.path.exists(input_path):
        shutil.make_archive(filename, 'zip', input_path)
        print("Zipped {} into {}".format(input_path, filename + ".zip"))


def main(zip_name="Element_Documentation"):
    proj_dir = os.getcwd()
    doc_path = os.path.join(proj_dir, "build/doc/html")

    if os.path.exists(doc_path):
        # Create the zip file for element documentation and put in the project directory
        create_zip_file(doc_path, zip_name)
    else:
        raise Exception('The \'{}\' path does not exist. Has the documentation been built?'.format(doc_path))


if __name__ == "__main__":
    if len(sys.argv) == 1:
        main()
    else:
        main(sys.argv[1])

import shutil
import os


def remove_dir(path):
    if os.path.isdir(path):
        shutil.rmtree(path)
        print("Previous version of {} has been removed.".format(path))
    else:
        print("{} does not exist, no need to remove.").format(path)

def copy_dir(src_path, dest_path):
    dest = shutil.copytree(src_path, dest_path)
    print("{} has been copied to {} successfully.".format(src_path, dest_path))


def main():
    proj_dir = os.getcwd()
    doc_path = os.path.join(proj_dir, "build/doc/html")
    dest_path = os.path.dirname(proj_dir) + "/docs/"

    # First remove the docs directory and all files
    remove_dir(dest_path)

    # Then copy the built docs into its location
    copy_dir(doc_path, dest_path)


if __name__ == "__main__":
    main()

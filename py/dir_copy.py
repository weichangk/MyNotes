import os
import shutil
import sys

def copy_files(src_dir, dest_dir):
    if not os.path.exists(dest_dir):
        shutil.copytree(src_dir, dest_dir)
        print(f"Created directory: {dest_dir}")
    else:
        for item in os.listdir(src_dir):
            s = os.path.join(src_dir, item)
            d = os.path.join(dest_dir, item)
            try:
                if os.path.isdir(s):
                    copy_files(s, d)
                else:
                    if os.path.exists(d):
                        print(f"Warning: {d} already exists. Overwriting...")
                    shutil.copy2(s, d)
                    print(f"Copied: {s} to {d}")
            except Exception as e:
                print(f"Error copying {s} to {d}: {e}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python dir_copy.py <src_dir> <dest_dir>")
    else:
        src_dir = os.path.join(os.path.dirname(__file__), sys.argv[1])
        dest_dir = os.path.join(os.path.dirname(__file__), sys.argv[2])
        
        if not os.path.exists(src_dir):
            print(f"Source directory '{src_dir}' does not exist.")
            sys.exit(1)
        
        if not os.path.exists(dest_dir):
            print(f"Destination directory '{dest_dir}' does not exist. Creating it...")
            os.makedirs(dest_dir)

        copy_files(src_dir, dest_dir)
        print(f"Completed copying from {src_dir} to {dest_dir}")

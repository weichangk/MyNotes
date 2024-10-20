import os
import shutil
import sys

def copy_dylib_files(src_dir, dest_dir):
    src_dir = os.path.join(os.path.dirname(__file__), src_dir)
    dest_dir = os.path.join(os.path.dirname(__file__), dest_dir)
    
    if not os.path.exists(src_dir):
        print(f"Source directory '{src_dir}' does not exist.")
        return
    
    if not os.path.exists(dest_dir):
        print(f"Destination directory '{dest_dir}' does not exist. Creating it...")
        os.makedirs(dest_dir)

    for file in os.listdir(src_dir):
        if file.endswith('.dylib'):
            src_file = os.path.join(src_dir, file)
            dest_file = os.path.join(dest_dir, file)
            try:
                if os.path.exists(dest_file):
                    print(f"Warning: {dest_file} already exists. Overwriting...")
                    # overwrite = input(f"{dest_file} already exists. Overwrite? (y/n): ")
                    # if overwrite.lower() != 'y':
                    #     print(f"Skipped: {dest_file}")
                    #     continue
                
                shutil.copy2(src_file, dest_file)
                print(f'Copied: {src_file} to {dest_file}')
            except Exception as e:
                print(f"Error copying {src_file} to {dest_file}: {e}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python dylib_copy.py <src_dir> <dest_dir>")
    else:
        src_dir = sys.argv[1]
        dest_dir = sys.argv[2]
        copy_dylib_files(src_dir, dest_dir)

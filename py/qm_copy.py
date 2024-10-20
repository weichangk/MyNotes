import os
import shutil
import sys

def get_language_dirs(lang_code):
    if lang_code == 'es':
        return ['esm', 'esp']
    return [lang_code]

def copy_qm_files(src_dir, dest_dir):
    src_dir = os.path.join(os.path.dirname(__file__), src_dir)
    dest_dir = os.path.join(os.path.dirname(__file__), dest_dir)
    
    if not os.path.exists(src_dir):
        print(f"Source directory '{src_dir}' does not exist.")
        return
    
    if not os.path.exists(dest_dir):
        print(f"Destination directory '{dest_dir}' does not exist. Creating it...")
        os.makedirs(dest_dir)

    files_copied = 0

    for filename in os.listdir(src_dir):
        if filename.endswith('.qm'):
            lang_code = filename.split('_')[-1].replace('.qm', '')
            language_dirs = get_language_dirs(lang_code)

            for lang_dir in language_dirs:
                full_lang_dir = os.path.join(dest_dir, lang_dir)
                if not os.path.exists(full_lang_dir):
                    os.makedirs(full_lang_dir)

                source_file = os.path.join(src_dir, filename)
                target_file = os.path.join(full_lang_dir, filename)

                try:
                    if os.path.exists(target_file):
                        print(f"Warning: {target_file} already exists. Overwriting...")
                        # overwrite = input(f"{target_file} already exists. Overwrite? (y/n): ")
                        # if overwrite.lower() != 'y':
                        #     print(f"Skipped: {target_file}")
                        #     continue
                    
                    shutil.copy2(source_file, target_file)
                    print(f"Copied: {source_file} to {target_file}")
                    files_copied += 1
                except Exception as e:
                    print(f"Error copying {source_file} to {target_file}: {e}")

    print(f"Total files copied: {files_copied}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python qm_copy.py <src_dir> <dest_dir>")
    else:
        src_dir = sys.argv[1]
        dest_dir = sys.argv[2]
        copy_qm_files(src_dir, dest_dir)

import os
import sys
import subprocess

def main(folder_name, lang_suffix=None):
    folder = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', folder_name))
    headers = []
    srcs = []

    for root, _, files in os.walk(folder):
        for file in files:
            if file.endswith(('.h', '.hpp')):
                headers.append(os.path.join(root, file))
            elif file.endswith(('.cpp', '.cc')):
                srcs.append(os.path.join(root, file))

    outfolder = os.path.join(folder, 'lang')
    os.makedirs(outfolder, exist_ok=True)

    if lang_suffix is None:
        for file in os.listdir(folder):
            if file.endswith('.ts'):
                ts_file = os.path.join(outfolder, file)
                print(f'lupdate {file}')
                subprocess.run(['lupdate', '-recursive'] + headers + srcs + ['-ts', ts_file])
    else:
        ts_file = os.path.join(outfolder, f'{folder_name}_{lang_suffix}.ts')
        print(f'lupdate {folder_name}_{lang_suffix}.ts')
        subprocess.run(['lupdate', '-recursive'] + headers + srcs + ['-ts', ts_file])

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python script.py <folder_name> [lang_suffix]")
    else:
        lang_suffix = sys.argv[2] if len(sys.argv) > 2 else None
        main(sys.argv[1], lang_suffix)

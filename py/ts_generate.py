import os
import sys
import subprocess

def main(src_dir, langName=None):
    folder = os.path.join(os.path.dirname(__file__), src_dir)
    headers = []
    srcs = []

    for root, _, files in os.walk(folder):
        for file in files:
            if file.endswith(('.h', '.hpp')):
                headers.append(os.path.join(root, file))
            elif file.endswith(('.cpp', '.cc')):
                srcs.append(os.path.join(root, file))

    if langName is None:
        ts_files = [f for f in os.listdir(folder) if f.endswith('.ts')]
        if not ts_files:
            print("No .ts files found in the specified directory.")
            return
        
        for file in ts_files:
            ts_file = os.path.join(folder, file)
            print(f'Running: lupdate {ts_file}')
            result = subprocess.run(['lupdate', '-recursive'] + headers + srcs + ['-ts', ts_file])
            if result.returncode != 0:
                print(f'Error updating {ts_file}')
    else:
        ts_file = os.path.join(folder, f'{langName}.ts')
        if not os.path.exists(ts_file):
            print(f"Error: The file '{ts_file}' does not exist.")
            return
        
        print(f'Running: lupdate {ts_file}')
        result = subprocess.run(['lupdate', '-recursive'] + headers + srcs + ['-ts', ts_file])
        if result.returncode != 0:
            print(f'Error updating {ts_file}')

    print(f'Found {len(headers)} header files and {len(srcs)} source files.')

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python script.py <src_dir> [langName]")
    else:
        langName = sys.argv[2] if len(sys.argv) > 2 else None
        main(sys.argv[1], langName)

import os
import sys
import subprocess

def main(src_dir, langName=None):
    folder = os.path.join(os.path.dirname(__file__), src_dir)

    if not os.path.exists(folder):
        print(f"Error: The directory '{folder}' does not exist.")
        return

    if langName is None:
        for root, _, files in os.walk(folder):
            for file in files:
                if file.endswith('.ts'):
                    ts_file = os.path.join(root, file)
                    print(f'Running: lrelease {ts_file}')
                    result = subprocess.run(['lrelease', ts_file, ts_file[:-3] + '.qm'])
                    if result.returncode == 0:
                        print(f'Successfully created {ts_file[:-3] + ".qm"}')
                    else:
                        print(f'Error processing {ts_file}')
    else:
        ts_file = os.path.join(folder, f'{langName}.ts')
        if not os.path.exists(ts_file):
            print(f"Error: The file '{ts_file}' does not exist.")
            return
        print(f'Running: lrelease {ts_file}')
        result = subprocess.run(['lrelease', ts_file, f'{langName}.qm'])
        if result.returncode == 0:
            print(f'Successfully created {langName}.qm')
        else:
            print(f'Error processing {ts_file}')

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python script.py <src_dir> [langName]")
    else:
        langName = sys.argv[2] if len(sys.argv) > 2 else None
        main(sys.argv[1], langName)

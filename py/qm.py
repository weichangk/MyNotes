import os
import sys
import subprocess

def main(folder_name, lang_suffix=None):
    folder = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', folder_name, 'lang'))
    # debug = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'bin', 'x64', 'Debug', 'lang', lang_suffix))
    # release = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'bin', 'x64', 'Release', 'lang', lang_suffix))
    # res = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', folder_name, 'res', 'lang', lang_suffix))

    os.chdir(folder)

    if lang_suffix is None:
        for root, _, files in os.walk(folder):
            for file in files:
                if file.endswith('.ts'):
                    ts_file = os.path.join(root, file)
                    print(f'lrelease {file}')
                    subprocess.run(['lrelease', ts_file, ts_file[:-3] + '.qm'])
    else:
        ts_file = f'{folder_name}_{lang_suffix}.ts'
        print(f'lrelease {ts_file}')
        subprocess.run(['lrelease', ts_file, f'{folder_name}_{lang_suffix}.qm'])

        # os.makedirs(debug, exist_ok=True)
        # os.makedirs(release, exist_ok=True)
        # os.makedirs(res, exist_ok=True)

        qm_file = f'{folder_name}_{lang_suffix}.qm'
        # subprocess.run(['copy', '/B', qm_file, os.path.join(debug, qm_file)], shell=True)
        # subprocess.run(['copy', '/B', qm_file, os.path.join(release, qm_file)], shell=True)
        # subprocess.run(['copy', '/B', qm_file, os.path.join(res, qm_file)], shell=True)

        # os.remove(qm_file)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python script.py <folder_name> [lang_suffix]")
    else:
        lang_suffix = sys.argv[2] if len(sys.argv) > 2 else None
        main(sys.argv[1], lang_suffix)

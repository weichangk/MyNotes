import os
import sys

def list_files(directory):
    output_file = os.path.join(directory, "output.txt")
    with open(output_file, 'w') as f:
        for _, _, files in os.walk(directory):
            for file in files:
                if file == "output.txt":
                    continue
                f.write(f"<file>Resources/light/{file}</file>\n")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("请提供要遍历的目录路径。")
        print("用法: python generate_file_list.py <directory_path>")
    else:
        directory_path = sys.argv[1]
        if not os.path.isdir(directory_path):
            print(f"目录 '{directory_path}' 不存在。")
        else:
            list_files(directory_path)
            print(f"文件列表已生成在 {directory_path}/output.txt")

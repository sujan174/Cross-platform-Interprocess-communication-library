import os

def print_directory_structure(path, indent=''):
    """Recursively prints the directory structure."""
    try:
        items = os.listdir(path)
    except PermissionError:
        print(f"{indent} [Permission Denied] {os.path.basename(path)}")
        return

    for item in items:
        full_path = os.path.join(path, item)
        if os.path.isdir(full_path):
            print(f"{indent} 📁 {item}")
            print_directory_structure(full_path, indent + '    ')
        else:
            print(f"{indent}>{item}")

if __name__ == "__main__":
    path = input("Enter the path to the directory: ")
    if os.path.exists(path):
        print_directory_structure(path)
    else:
        print("Invalid path.")
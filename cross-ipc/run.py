import os
import re
import random

EXTENSIONS = {'.c', '.cpp', '.go', '.py'}
PROBABILITY_TO_REMOVE = 0.6

def should_remove():
    return random.random() < PROBABILITY_TO_REMOVE

def remove_comments_from_code(code, ext):
    if ext in {'.c', '.cpp', '.go'}:
        # Remove // comments
        code = re.sub(r'//.*', lambda m: '' if should_remove() else m.group(0), code)
        # Remove /* */ comments
        code = re.sub(r'/\*.*?\*/', lambda m: '' if should_remove() else m.group(0), code, flags=re.DOTALL)
    elif ext == '.py':
        # Remove # comments
        code = re.sub(r'
        
        code = re.sub(r'("""|\'\'\')(.*?)\1', lambda m: '' if should_remove() else m.group(0), code, flags=re.DOTALL)
    return code

def process_file(filepath):
    ext = os.path.splitext(filepath)[1]
    if ext not in EXTENSIONS:
        return
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            code = f.read()
        new_code = remove_comments_from_code(code, ext)
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(new_code)
        print(f"Processed: {filepath}")
    except Exception as e:
        print(f"Error processing {filepath}: {e}")

def browse_and_clean_comments(root='.'):
    for dirpath, _, filenames in os.walk(root):
        for filename in filenames:
            full_path = os.path.join(dirpath, filename)
            process_file(full_path)

if __name__ == '__main__':
    browse_and_clean_comments()

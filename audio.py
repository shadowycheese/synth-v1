import re
import os

def convert_to_voice_class(input_text, class_name="Voice"):
    # Regex for Audio Objects and Connections
    obj_pattern = r"(Audio\w+)\s+(\w+);"
    patch_pattern = r"AudioConnection\s+(\w+)\(([\w, \d]+)\);"

    objects = re.findall(obj_pattern, input_text)
    patches = re.findall(patch_pattern, input_text)

    if not objects:
        return "// No audio objects found in audio.txt."

    header = [
        f"#ifndef {class_name.upper()}_H",
        f"#define {class_name.upper()}_H",
        "",
        "#include <Audio.h>",
        "",
        f"class {class_name} {{",
        "public:"
    ]
    
    # 1. Members (Actual objects, not pointers)
    header.append("  // --- Audio Objects ---")
    for obj_type, obj_name in objects:
        header.append(f"  {obj_type} {obj_name};")
    
    header.append("\n  // --- Patch Cords ---")
    for patch_name, _ in patches:
        header.append(f"  AudioConnection {patch_name};")

    # 2. Constructor with Member Initializer List
    header.append(f"\n  {class_name}() :")
    
    # Generate the list of initializers
    init_list = []
    for i, (patch_name, args) in enumerate(patches):
        # Add a comma to all but the last entry
        suffix = "," if i < len(patches) - 1 else ""
        init_list.append(f"    {patch_name}({args}){suffix}")
    
    header.extend(init_list)
    header.append("  {} // Empty constructor body");
    
    header.append("};")
    header.append(f"\n#endif")

    return "\n".join(header)

def main():
    if not os.path.exists("audio.txt"):
        print("Error: audio.txt not found.")
        return

    with open("audio.txt", "r") as f:
        input_text = f.read()

    cpp_code = convert_to_voice_class(input_text)

    with open("Voice.h", "w") as f:
        f.write(cpp_code)
    print("Success! Generated Voice.h with Initializer List.")

if __name__ == '__main__':
    main()
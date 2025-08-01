import sys, os, json, re

labelfile = "temp.txt"

if len(sys.argv) != 3:
    print("Error: Invalid number of arguments")
    print("Usage: python3 update_localization_server.py <old_file> <output_file_name>")
    sys.exit(-1)

old_file = sys.argv[1]
output_file_name = sys.argv[2]

unique_labels = set()  # Use set() to avoid duplicates


def get_localized(directory, suffix, tpattern):
    pattern = re.compile(tpattern, re.IGNORECASE)
    for root, dirs, files in os.walk(directory):
        for filename in files:
            if filename.endswith(suffix):
                # Get full file path
                file_path = os.path.join(root, filename)
                # Open and read file content
                with open(file_path, "r", encoding="utf-8") as file:
                    content = file.read()
                    # Find all matches
                    matches = pattern.findall(content)
                    # Add all matched strings to the list
                    for found in matches:
                        if (
                            found not in unique_labels
                        ):  # Check if the string is already in the set
                            unique_labels.add(found + "\n")  # Add to the set

    with open(labelfile, "w") as fw:
        fw.writelines(sorted(unique_labels))  # Sort and write the labels to the file


get_localized("src/game/server/", ".cpp", 'Localize\\("(.+?)"')
get_localized("src/game/server/", ".cpp", 'SendChatTarget\\([^,]*, "(.+?)"')
get_localized("src/game/server/", ".cpp", 'SendBroadcast\\("(.+?)"')
get_localized("src/game/server/", ".cpp", 'SendBroadcastFormat\\([^,]*, [^,]*, "(.+?)"')
get_localized("data/server/gamevotes", ".vot", "name: (.*)")
get_localized("data/server/gamevotes", ".vot", "description: (.*)")


with open(labelfile, "r") as file:
    lines = file.readlines()

translation_list = [{"key": line.strip(), "value": line.strip()} for line in lines]
translation_dict = {"translation": translation_list}

with open(output_file_name, "w") as output_file:
    json.dump(translation_dict, output_file, indent=4)


def unicode_to_chinese_text(text):
    return re.sub(r"\\u([0-9a-fA-F]{4})", lambda m: chr(int(m.group(1), 16)), text)


with open(old_file, "r") as file1:
    file1_data = json.load(file1)
    file1_data = json.loads(unicode_to_chinese_text(json.dumps(file1_data)))

with open(output_file_name, "r") as file2:
    file2_data = json.load(file2)
    file2_data = json.loads(unicode_to_chinese_text(json.dumps(file2_data)))

translation_map = {item["key"]: item["value"] for item in file1_data["translation"]}
final_translation = []

for item in file2_data["translation"]:
    key = item["key"]
    # If the key exists in file 1, use the value from file 1, otherwise use the value from file 2
    value = translation_map.get(key, item["value"])
    final_translation.append({"key": key, "value": value})

# Create the final dictionary structure
output_data = {"translation": final_translation}

with open(output_file_name, "w") as output_file:
    json.dump(output_data, output_file, indent=4)


def unicode_to_chinese_file(file_path):
    unicode_re = re.compile(r"\\u([0-9a-fA-F]{4})")

    with open(file_path, "r", encoding="utf-8") as file:
        content = file.read()

    def unicode_replacer(match):
        return chr(int(match.group(1), 16))

    chinese_content = unicode_re.sub(unicode_replacer, content)

    with open(file_path, "w", encoding="utf-8") as file:
        file.write(chinese_content)


unicode_to_chinese_file(output_file_name)

with open(output_file_name, "r", encoding="utf-8") as input_file:
    data = json.load(input_file)

# Sort all translation items by key
sorted_items = sorted(data["translation"], key=lambda x: x["key"])
different_items = [item for item in sorted_items if item["key"] != item["value"]]
same_items = [item for item in sorted_items if item["key"] == item["value"]]
sorted_translation = same_items + different_items
data["translation"] = sorted_translation

with open(output_file_name, "w", encoding="utf-8") as output_file:
    json.dump(data, output_file, ensure_ascii=False, indent=4)

print(f"Output has been written to {output_file_name}")

os.remove("temp.txt")

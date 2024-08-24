import sys
import os
import fnmatch
import json
import re

labelfile = "temp.txt"

directory = "src/game/server/"

old_file = sys.argv[1]
output_file_name = sys.argv[2]

unique_labels = []

for root, dirs, files in os.walk(directory):
    for filename in fnmatch.filter(files, "*.cpp"):
        with open(os.path.join(root, filename), "r") as file:
            lines = file.readlines()
            for line in lines:
                f = line.find(r'_("')
                if f != -1:
                    end_quote = line.find(r'")', f)
                    if end_quote != -1:
                        label = line[f + 3 : end_quote]
                        unique_labels.append(label + "\n")

# Load gamevotes
for file_name in os.listdir("data/server/gamevotes/"):
    file_path = os.path.join("data/server/gamevotes/", file_name)

    # 确保是文件而不是文件夹
    if os.path.isfile(file_path):
        with open(file_path, "r") as file:
            text = file.read()

        # 使用正则表达式匹配name和description字段
        pattern = re.compile(r"^(name:|description:)\s+(.*)", re.MULTILINE)

        # 找到所有匹配项
        matches = pattern.findall(text)

        # 提取name和description的值
        name = matches[0][1] if matches and matches[0][0] == "name:" else None
        description = (
            matches[1][1]
            if matches and len(matches) > 1 and matches[1][0] == "description:"
            else None
        )

        if name != None:
            unique_labels.append(name + "\n")

        if description != None:
            unique_labels.append(description + "\n")

with open(labelfile, "w") as fw:
    fw.writelines(set(unique_labels))

with open(labelfile, "r") as file:
    lines = file.readlines()

# 创建一个列表，用于存储每一行的键值对
translation_list = [{"key": line.strip(), "value": line.strip()} for line in lines]

# 创建一个字典，包含所有键值对
translation_dict = {"translation": translation_list}

# 将字典写入到输出文件中，格式化为JSON
with open(output_file_name, "w") as output_file:
    json.dump(translation_dict, output_file, indent=4)


# 将Unicode编码的文本转换成汉字
def unicode_to_chinese(text):
    # 使用正则表达式匹配所有的Unicode转义序列
    return re.sub(r"\\u([0-9a-fA-F]{4})", lambda m: chr(int(m.group(1), 16)), text)


# 读取文件1和文件2的内容
with open(old_file, "r") as file1:
    file1_data = json.load(file1)
    # 转换文件1中的Unicode文本
    file1_data = json.loads(unicode_to_chinese(json.dumps(file1_data)))

with open(output_file_name, "r") as file2:
    file2_data = json.load(file2)
    # 转换文件2中的Unicode文本
    file2_data = json.loads(unicode_to_chinese(json.dumps(file2_data)))

# 创建一个字典，用于将文件1中的键映射到值
translation_map = {item["key"]: item["value"] for item in file1_data["translation"]}

# 创建一个新列表，用于存储最终的翻译
final_translation = []

# 遍历文件2中的键，并从映射中获取对应的值
for item in file2_data["translation"]:
    key = item["key"]
    # 如果文件1中存在对应的键，则使用文件1中的值，否则使用文件2中的值
    value = translation_map.get(key, item["value"])
    final_translation.append({"key": key, "value": value})

# 创建最终的字典结构
output_data = {"translation": final_translation}

# 将最终的字典写入到输出文件中
with open(output_file_name, "w") as output_file:
    json.dump(output_data, output_file, indent=4)


def unicode_to_chinese(file_path):
    # 正则表达式，用于匹配Unicode编码
    unicode_re = re.compile(r"\\u([0-9a-fA-F]{4})")

    # 读取文件内容
    with open(file_path, "r", encoding="utf-8") as file:
        content = file.read()

    # 将文件中的Unicode编码转换为汉字
    def unicode_replacer(match):
        return chr(int(match.group(1), 16))

    # 使用正则表达式替换文件内容中的Unicode编码
    chinese_content = unicode_re.sub(unicode_replacer, content)

    # 将转换后的内容写回文件或输出到控制台
    with open(file_path, "w", encoding="utf-8") as file:
        file.write(chinese_content)


# 使用函数
unicode_to_chinese(output_file_name)

# 读取输入文件的内容
with open(output_file_name, "r", encoding="utf-8") as input_file:
    data = json.load(input_file)

# 将翻译项分成两组：一组是key和value不一致的，另一组是key和value一致的
different_items = [item for item in data["translation"] if item["key"] != item["value"]]
same_items = [item for item in data["translation"] if item["key"] == item["value"]]

# 将两组项合并，把key和value一致的项放到末尾
sorted_translation = different_items + same_items

# 更新数据
data["translation"] = sorted_translation

# 将排序后的数据写入输出文件
with open(output_file_name, "w", encoding="utf-8") as output_file:
    json.dump(data, output_file, ensure_ascii=False, indent=4)

print(f"Output has been written to {output_file_name}")

os.remove("temp.txt")
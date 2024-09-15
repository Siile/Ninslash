import sys
import os
import fnmatch
import json
import re

labelfile = "temp.txt"

old_file = sys.argv[1]
output_file_name = sys.argv[2]

unique_labels = set()  # 使用集合来避免重复

def help_localize(directory, suffix, tpattern):
    pattern = re.compile(tpattern, re.IGNORECASE)
    for root, dirs, files in os.walk(directory):
        for filename in files:
            if filename.endswith(suffix):
                # 获取完整文件路径
                file_path = os.path.join(root, filename)
                # 打开并读取文件内容
                with open(file_path, 'r', encoding='utf-8') as file:
                    content = file.read()
                    # 查找所有匹配项
                    matches = pattern.findall(content)
                    # 将所有匹配的字符串添加到列表中
                    for found in matches:
                        if found not in unique_labels:  # 检查是否重复
                            unique_labels.add(found + "\n")  # 添加到集合中

    with open(labelfile, "w") as fw:  # 使用 "w" 而不是 "a" 来覆盖旧文件
        fw.writelines(sorted(unique_labels))  # 对标签进行排序并写入文件

help_localize("src/game/server/", '.cpp', 'Localize\\("(.+?)"')
help_localize("src/game/server/", '.cpp', 'SendChatTarget\\([^,]*, "(.+?)"')
help_localize("src/game/server/", '.cpp', 'SendBroadcast\\("(.+?)"')
help_localize("src/game/server/", '.cpp', 'SendBroadcastFormat\\([^,]*, [^,]*, "(.+?)"')
help_localize("data/server/gamevotes", '.vot', 'name: (.*)')
help_localize("data/server/gamevotes", '.vot', 'description: (.*)')



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

# 将所有翻译项按key进行排序
sorted_items = sorted(data["translation"], key=lambda x: x['key'])

# 找出所有key和value不一致的项，并将它们移到列表的末尾
different_items = [item for item in sorted_items if item["key"] != item["value"]]
same_items = [item for item in sorted_items if item["key"] == item["value"]]

# 将排序后的项合并，确保不一致的项在末尾
sorted_translation = same_items + different_items

# 更新数据
data["translation"] = sorted_translation

# 将排序后的数据写入输出文件
with open(output_file_name, "w", encoding="utf-8") as output_file:
    json.dump(data, output_file, ensure_ascii=False, indent=4)

print(f"Output has been written to {output_file_name}")

os.remove("temp.txt")
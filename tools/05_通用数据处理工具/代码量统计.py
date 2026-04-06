import os

# 定义需要统计的文件夹列表
TARGET_DIRS = ['code', 'code1', 'user', 'docs', 'tools']

# 定义每个目录需要统计的文件后缀
DIR_EXTENSIONS_MAP = {
    'code':   {'.c', '.h'},
    'code1':  {'.c', '.h'},
    'user':   {'.c', '.h'},
    'docs':   {'.md'},
    'tools':  {'.html', '.py'}
}

# 定义汇总分组的映射关系
# Key: 分组名称, Value: 包含的目录列表
GROUP_MAP = {
    'C代码总计1': ['code', 'code1', 'user'],
    '工具和文档总计2': ['docs', 'tools']
}

def is_target_file(filename, current_dir):
    """
    根据当前所在目录，判断文件是否为需要统计的类型
    """
    # 获取当前目录允许的后缀集合，如果未定义则默认为空（不统计）
    allowed_exts = DIR_EXTENSIONS_MAP.get(current_dir, set())
    return os.path.splitext(filename)[1].lower() in allowed_exts

def analyze_file(filepath):
    """
    分析单个文件，返回 (总行数, 代码行, 注释行, 空行)
    注意：对于 .md 和 .html，"代码行"即指非空非注释的有效内容行。
    """
    total_lines = 0
    code_lines = 0
    comment_lines = 0
    blank_lines = 0
    
    # 状态标记，用于处理多行注释 /* ... */
    in_block_comment = False

    try:
        # 使用 errors='ignore' 防止编码问题导致崩溃
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            for line in f:
                total_lines += 1
                stripped_line = line.strip()
                
                if not stripped_line:
                    # 空行
                    blank_lines += 1
                    continue
                
                # --- 以下为针对 C/C++ (.c, .h) 的注释处理逻辑 ---
                # 如果文件不是 .c 或 .h，跳过复杂的注释块检测，直接视为代码行
                # (Markdown 和 HTML 的注释结构不同，这里简化处理，视非空行为代码)
                if not (filepath.endswith('.c') or filepath.endswith('.h')):
                    code_lines += 1
                    continue

                # 处理多行注释状态
                if in_block_comment:
                    comment_lines += 1
                    if "*/" in stripped_line:
                        in_block_comment = False
                    continue
                
                # 检查是否进入多行注释
                if "/*" in stripped_line:
                    comment_lines += 1
                    # 检查同一行是否结束注释 */
                    if "*/" not in stripped_line:
                        in_block_comment = True
                    continue
                
                # 检查单行注释 //
                if stripped_line.startswith("//"):
                    comment_lines += 1
                    continue
                
                # 剩下的情况视为代码行
                code_lines += 1
                
    except Exception as e:
        print(f"读取文件出错: {filepath}, 错误: {e}")

    return total_lines, code_lines, comment_lines, blank_lines

def main():
    # 用于存储每个目录的统计数据
    # 结构: { 'dir_name': {'files': int, 'total': int, 'code': int, 'comment': int, 'blank': int} }
    dir_stats = {}

    print(f"开始统计目录: {', '.join(TARGET_DIRS)}\n")
    print(f"{'目录':<15} | {'文件数':<6} | {'总行数':<8} | {'代码行':<8} | {'注释行':<8} | {'空行':<8}")
    print("-" * 75)

    # 1. 遍历所有目录，收集基础数据
    for target_dir in TARGET_DIRS:
        if not os.path.exists(target_dir):
            print(f"警告: 目录 '{target_dir}' 不存在，跳过。")
            # 初始化为0，防止后续汇总报错
            dir_stats[target_dir] = {'files': 0, 'total': 0, 'code': 0, 'comment': 0, 'blank': 0}
            continue
        
        current_stats = {'files': 0, 'total': 0, 'code': 0, 'comment': 0, 'blank': 0}
        
        for root, dirs, files in os.walk(target_dir):
            for file in files:
                if is_target_file(file, target_dir):
                    file_path = os.path.join(root, file)
                    t, c, com, b = analyze_file(file_path)
                    
                    current_stats['files'] += 1
                    current_stats['total'] += t
                    current_stats['code'] += c
                    current_stats['comment'] += com
                    current_stats['blank'] += b
        
        dir_stats[target_dir] = current_stats

        # 打印当前目录结果
        print(f"{target_dir:<15} | {current_stats['files']:<6} | {current_stats['total']:<8} | {current_stats['code']:<8} | {current_stats['comment']:<8} | {current_stats['blank']:<8}")

    print("-" * 75)

    # 2. 计算并打印分组汇总
    grand_total_files = 0
    grand_total_lines = 0
    grand_code_lines = 0
    grand_comment_lines = 0
    grand_blank_lines = 0

    for group_name, dir_list in GROUP_MAP.items():
        group_files = 0
        group_total = 0
        group_code = 0
        group_comment = 0
        group_blank = 0
        
        for d in dir_list:
            if d in dir_stats:
                s = dir_stats[d]
                group_files += s['files']
                group_total += s['total']
                group_code += s['code']
                group_comment += s['comment']
                group_blank += s['blank']
        
        # 累加到总计
        grand_total_files += group_files
        grand_total_lines += group_total
        grand_code_lines += group_code
        grand_comment_lines += group_comment
        grand_blank_lines += group_blank

        print(f"{group_name:<15} | {group_files:<6} | {group_total:<8} | {group_code:<8} | {group_comment:<8} | {group_blank:<8}")

    # 3. 打印最终总计
    print("=" * 75)
    print(f"{'总计':<15} | {grand_total_files:<6} | {grand_total_lines:<8} | {grand_code_lines:<8} | {grand_comment_lines:<8} | {grand_blank_lines:<8}")

if __name__ == '__main__':
    main()

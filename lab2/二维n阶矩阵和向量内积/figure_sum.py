import matplotlib.pyplot as plt
import os
import re

# 配置参数
data_files = ['way2_normal_algo.txt', 'way2_unroll.txt']  # 两个文件
colors = ['royalblue', 'darkorange']     # 两组数据的颜色
markers = ['o', 's']                     # 两组数据的标记样式
labels = ['cache', 'cache_unroll']   # 图例标签

plt.figure(figsize=(12, 6))

# 更灵活的正则表达式，匹配两种格式：
# n = 10; way1_Elapsed time: 0.212 us
# n = 10; way2_Elapsed time: 0.156 us
pattern = re.compile(r'n\s*=\s*(\d+);\s*way(1|2)[_ ]?Elapsed time:\s*([\d.]+)\s*us')

# 存储所有数据的字典
data_dict = {}

# 遍历所有数据文件
for file_idx, filename in enumerate(data_files):
    try:
        # 检查文件是否存在
        if not os.path.exists(filename):
            print(f"错误: 文件 {filename} 未找到，跳过")
            continue
            
        # 读取数据文件
        with open(filename, 'r') as f:
            file_content = f.readlines()
        
        # 提取数据
        n_values = []
        elapsed_times = []
        for line in file_content:
            line = line.strip()
            match = pattern.match(line)
            if match:
                n = int(match.group(1))
                way = match.group(2)  # 捕获way1或way2
                time = float(match.group(3))
                n_values.append(n)
                elapsed_times.append(time)
                print(f"成功匹配: {line}")  # 调试输出
            else:
                print(f"未能匹配: {line}")   # 调试输出
        
        if not n_values:
            print(f"警告: 文件 {filename} 中没有找到有效数据")
            continue
        
        # 存储数据以便后续排序
        data_dict[file_idx] = {
            'n': n_values,
            'time': elapsed_times,
            'filename': filename,
            'way': 'way1' if 'way1' in filename else 'way2'
        }
        
    except Exception as e:
        print(f"处理文件 {filename} 时出错: {str(e)}")
        continue

# 检查是否获取到数据
if not data_dict:
    print("错误: 没有有效数据可绘制")
    exit()

# 对每个数据集按n值排序并绘制
for file_idx, data in data_dict.items():
    # 将n值和对应时间一起排序
    sorted_data = sorted(zip(data['n'], data['time']), key=lambda x: x[0])
    sorted_n = [x[0] for x in sorted_data]
    sorted_time = [x[1] for x in sorted_data]
    
    # 确定标签 - 根据文件名或way值
    label = labels[file_idx % len(labels)]
    
    print(f"绘制数据: 文件 {data['filename']}, way {data['way']}, 标签 {label}")  # 调试输出
    
    # 绘制当前数据集
    plt.plot(sorted_n, sorted_time, 
            color=colors[file_idx % len(colors)],
            marker=markers[file_idx % len(markers)],
            label=label,
            linestyle='-',
            linewidth=1)
    
    # 添加数据标签（可选）
    if len(sorted_n) <= 20:
        for n, time in zip(sorted_n, sorted_time):
            plt.text(n, time, f'{time:.3f} μs', 
                     ha='center', va='bottom', 
                     fontsize=8, 
                     color=colors[file_idx % len(colors)])

# 美化图表
plt.title('Method Execution Time Comparison', fontsize=14)
plt.xlabel('n value', fontsize=12)
plt.ylabel('Elapsed Time (μs)', fontsize=12)
plt.grid(axis='y', linestyle='--', alpha=0.7)
plt.legend()
plt.tight_layout()

# 显示图表
plt.show()
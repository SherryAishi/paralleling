import matplotlib.pyplot as plt
import os
import re

# 配置参数
data_files = ['f1_normal_algo.txt', 'f1_unrolling.txt', 'f1_unrolling_plus.txt']  # 替换为你的实际文件名
colors = ['royalblue', 'darkorange', 'forestgreen']  # 每组数据的颜色
markers = ['o', 's', '^']  # 每组数据的标记样式
labels = ['normal_algo', 'unrolling_algo', 'unrolling_plus_algo']  # 图例标签

plt.figure(figsize=(12, 6))

# 修改后的正则表达式用于提取数据 - 匹配新的时间格式
pattern = re.compile(r'f\d+_Elapsed time: ([\d.]+) us')

# 遍历所有数据文件
for file_idx, filename in enumerate(data_files):
    try:
        # 读取数据文件
        with open(filename, 'r') as f:
            file_content = f.readlines()
        
        # 提取数据
        elapsed_times = []
        for line in file_content:
            match = pattern.match(line.strip())
            if match:
                # 提取时间值并转换为浮点数
                elapsed_times.append(float(match.group(1)))
        
        if not elapsed_times:
            print(f"警告: 文件 {filename} 中没有找到有效数据，跳过")
            continue
        
        indices = range(1, len(elapsed_times) + 1)  # x轴坐标
        
        # 绘制当前数据集
        plt.plot(indices, elapsed_times, 
                color=colors[file_idx % len(colors)],
                marker=markers[file_idx % len(markers)],
                label=f'{labels[file_idx % len(labels)]}',
                linestyle='-',
                linewidth=1)
        
        # 添加数据标签（可选，如果数据点很多可能会显得拥挤）
        if len(elapsed_times) <= 20:  # 只在数据点较少时添加标签
            for i, time in zip(indices, elapsed_times):
                plt.text(i, time, f'{time:.3f} μs', 
                         ha='center', va='bottom', 
                         fontsize=8, 
                         color=colors[file_idx % len(colors)])

    except FileNotFoundError:
        print(f"错误: 文件 {filename} 未找到，跳过")
    except Exception as e:
        print(f"处理文件 {filename} 时出错: {str(e)}")

# 美化图表
plt.title('Function Execution Time Measurements Comparison', fontsize=14)
plt.xlabel('logN', fontsize=12)
plt.ylabel('Elapsed Time (μs)', fontsize=12)
plt.grid(axis='y', linestyle='--', alpha=0.7)
plt.legend()
plt.tight_layout()

# 显示图表
plt.show()
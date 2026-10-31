#!/bin/bash
echo "====================================="
echo "=== Cache Lab 自动化演示脚本 ==="
echo "====================================="

echo -e "\n[1] 重新编译..."
make clean && make

echo -e "\n[2] 运行 Part A: 缓存模拟器测试 (目标 27分)"
./test-csim

echo -e "\n[3] 运行 Part B: 32x32 矩阵转置测试 (目标 Miss < 300)"
./test-trans -M 32 -N 32

echo -e "\n[4] 运行 Part B: 64x64 矩阵转置测试 (目标 Miss < 1300)"
./test-trans -M 64 -N 64

echo -e "\n[5] 运行 Part B: 61x67 矩阵转置测试 (目标 Miss < 2000)"
./test-trans -M 61 -N 67

#! /usr/bin/env python
# -*- coding: utf-8 -*-
# LinuxRelease/run.py

import os
import numpy as np

if __name__ == "__main__":
    # 遍历maps目录下所有地图（文件名为map0.txt、map1.txt...）
    file_lists = []
    for each_map in os.listdir("./maps"):
        if each_map.startswith("map"):
            file_lists.append(each_map)

    # 总分统计
    score_list1 = np.zeros(len(file_lists))
    score_list2 = np.zeros(len(file_lists))

    prog1 = "../build/main"
    prog2 = "Demo/SimpleDemo"

    for idx, each_map in enumerate(file_lists):
        # 将标准输出读入
        stdout_map1 = os.popen("./Robot -f -l ERR " +
                              prog1 + " -m maps/{}".format(each_map)).read()
        # json解析，找score字段
        score_map1 = stdout_map1.split("\"score\":")[1].split("}")[0]
        # print("地图{}的得分为：{}".format(each_map, score_map))
        score_list1[idx] = score_map1

        # 将标准输出读入
        stdout_map2 = os.popen("./Robot -f -l ERR " +
                              prog2 + " -m maps/{}".format(each_map)).read()
        # json解析，找score字段
        score_map2 = stdout_map2.split("\"score\":")[1].split("}")[0]
        # print("地图{}的得分为：{}".format(each_map, score_map))
        score_list2[idx] = score_map2

    # 计算平均分和总分
    print("user总分为：{}".format(np.sum(score_list1)))
    print("user平均分为：{}".format(np.mean(score_list1)))
    print("demo总分为：{}".format(np.sum(score_list2)))
    print("demo平均分为：{}".format(np.mean(score_list2)))

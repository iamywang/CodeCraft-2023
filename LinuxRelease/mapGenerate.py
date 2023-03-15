import numpy as np
import os


save_map_dir = "./maps/"
if not os.path.exists(save_map_dir):
    os.makedirs(save_map_dir)
###########################################
num_map = 100  # 一次性生成数量
###########################################
for _ in range(num_map):
    # init
    myMap = np.array([['.'] * 100 for _ in range(100)])
    # get robot random pos
    robot_x = np.random.randint(low=1, high=99, size=4)
    robot_y = np.random.randint(low=1, high=99, size=4)
    # set robot
    for i in range(4):
        myMap[robot_x[i]][robot_y[i]] = "A"
    while True:
        # how many workbench
        num_workBench = np.random.randint(low=10, high=51)
        num_only_producerBench = int(num_workBench * 0.5)
        num_only_consumerBench = int(num_workBench * 0.15)
        num_otherBench = num_workBench - num_only_producerBench - num_only_consumerBench
        # workbench type
        producerBench = np.random.randint(low=1, high=4, size=num_only_producerBench)
        otherBench = np.random.randint(low=4, high=8, size=num_otherBench)
        consumerBench = np.random.randint(low=8, high=10, size=num_only_consumerBench)
        if num_only_consumerBench == 1:
            consumerBench[0] = 9
        # check & set workbench
        if 1 not in producerBench or 2 not in producerBench or 3 not in producerBench:
            continue
        else:  # 1-3 exist
            if 4 not in otherBench or 5 not in otherBench or 6 not in otherBench:
                continue
            else:  # 4-6 exist
                if num_only_consumerBench >= 2 and (7 not in otherBench or 9 not in consumerBench):
                    continue
                else:
                    allBench = np.hstack((producerBench, otherBench, consumerBench))
                    print("工作站总数：{}".format(allBench.shape))
                    for benchType in allBench:
                        pos_x = np.random.randint(low=1, high=100)
                        pos_y = np.random.randint(low=1, high=100)
                        while myMap[pos_x][pos_y] == 'A':
                            pos_x = np.random.randint(low=1, high=100)
                            pos_y = np.random.randint(low=1, high=100)
                        myMap[pos_x][pos_y] = benchType
                    num_existedMap = len(os.listdir(save_map_dir))
                    with open(save_map_dir+"map{}.txt".format(num_existedMap), "w") as f:
                        for each_line in myMap:
                            for each_point in each_line:
                                f.write(str(each_point))
                            f.write("\n")
                    break
print("done")

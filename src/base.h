#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <stack>
#include <string>
#include <unistd.h>
#include <vector>

using namespace std;

#ifndef BASE_CLASSES
#define BASE_CLASSES

// 机器人信息: 机器人ID, 工作台ID, 物品类型, 时间价值系数, 碰撞价值系数, 角速度, 线速度, 朝向, 坐标
struct Robot {
    int id;
    int platform_id;
    int item_type;
    double time_value;
    double collision_value;
    double angular_velocity;
    pair<double, double> linear_velocity;
    double orientation;
    pair<double, double> position;

    // 到每个工作台的角速度，旋转时间，前进速度，前进时间
    vector<double> platform_distance;
    vector<double> platform_angular_velocity;
    vector<double> platform_rotate_frame;
    vector<double> platform_forward_velocity;
    vector<double> platform_forward_frame;

    // 到工作台的距离排序
    vector<int> platform_distance_sort_buy;
    vector<int> platform_distance_sort_sell;

    // 构造函数
    Robot(int id) {
        this->id = id;
    }
};

// 工作台信息: 工作台ID, 工作台类型, 工作台坐标, 剩余生产时间, 原材料状态, 产品状态
struct Platform {
    int id;
    pair<double, double> position;
    int remain_time;
    int material_state;
    int product_state;

    // 构造函数
    Platform(int id) {
        this->id = id;
    }
};

#endif

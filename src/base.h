#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
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

#define pi 3.14159265358979323846
#define fps 50
#define radius_without 0.45
#define radius_with 0.53
#define density 20
#define ignore_sub 1e-6

// 牵引力和力矩
#define force 250
#define torque 50

// 最大最小前进旋转速度
#define max_forward_speed 6
#define min_forward_speed -2
#define max_rotate_speed pi
#define min_rotate_speed -pi

// 可调整参数（好像真的有用）
// 1. 机器人是否在工作台判断距离（原始值为0.4）
double distance_within = 0.38;

// 2. 贪心策略中，时间系数的估计值
double time_coefficient = 0.9;

// 3. 帧数上限，解决购买后时间不足以出售的问题
double max_frames = 8980;

// 大地图 100*100 (0.5m*0.5m)
// vector<vector<char>> maps = vector<vector<char>>(100, vector<char>(100, '.'));

// 机器人
vector<Robot *> robots;
// map<pair<double, double>, int> robot_map;

// 工作台
vector<Platform *> platforms;
map<pair<double, double>, int> platform_map;

// 物品价格表
map<int, pair<int, int>> item_prices;

// 物品配方表
map<int, int> item_recipes;

// 当前帧每种物品的需求量（按照编号为1-7）
map<int, vector<int>> item_demand;
map<int, int> available_demand;

// 当前帧每种物品的提供量（按照编号为1-7）
map<int, vector<int>> item_supply;
map<int, vector<int>> item_pending;

// 当前帧已经被选择的工作台（购买阶段）
set<int> selected_platforms_buy;

// 当前帧已经被选择的工作台（出售阶段）
set<pair<int, int>> selected_platforms_sell;

double nine_numbers = 0;
double eight_numbers = 0;

// 读入地图
void readMapUntilOK() {
    int current_row = 0;
    int current_robot = 0;
    int current_platform = 0;
    char line[1024];
    while (fgets(line, sizeof line, stdin)) {
        if (line[0] == 'O' && line[1] == 'K') {
            for (int i = 0; i < 4; i++) {
                robots[i]->platform_distance = vector<double>(platforms.size(), 0);
                robots[i]->platform_angular_velocity = vector<double>(platforms.size(), 0);
                robots[i]->platform_rotate_frame = vector<double>(platforms.size(), 0);
                robots[i]->platform_forward_velocity = vector<double>(platforms.size(), 0);
                robots[i]->platform_forward_frame = vector<double>(platforms.size(), 0);
            }
            return;
        }

        // 保存地图信息到maps
        for (int i = 0; i < 100; i++) {
            // maps[current_row][i] = line[i];

            // 如果是工作台，保存工作台位置到platforms
            if (line[i] - '0' >= 1 && line[i] - '0' <= 9) {
                double center_x = double(i) * 0.5 + 0.25;
                double center_y = 50 - double(current_row) * 0.5 - 0.25;
                platforms.push_back(new Platform(line[i] - '0'));
                platforms[current_platform]->position = make_pair(center_x, center_y);
                platform_map.insert(make_pair(make_pair(center_x, center_y), current_platform));
                // cerr << "platform " << current_platform << " " << center_x << " " << center_y << endl;
                current_platform++;
            }

            if (line[i] - '0' == 8)
                eight_numbers++;
            if (line[i] - '0' == 9)
                nine_numbers++;

            // 如果是机器人，保存机器人位置到robots
            if (line[i] == 'A') {
                double center_x = double(i) * 0.5 + 0.25;
                double center_y = 50 - double(current_row) * 0.5 - 0.25;
                robots.push_back(new Robot(current_robot));
                robots[current_robot]->position = make_pair(center_x, center_y);
                // robot_map.insert(make_pair(make_pair(center_x, center_y), current_robot));
                // cerr << "robot " << current_robot << " " << center_x << " " << center_y << endl;
                current_robot++;
            }
        }
        current_row++;
    }
}

// 读入一帧信息
void readFrameUntilOK() {
    char line[1024];

    // 第一行: 帧ID和金钱
    fgets(line, sizeof line, stdin);
    int frame_id, money;
    sscanf(line, "%d %d", &frame_id, &money);

    item_demand = map<int, vector<int>>();
    available_demand = map<int, int>();
    item_supply = map<int, vector<int>>();
    item_pending = map<int, vector<int>>();
    selected_platforms_buy = set<int>();
    selected_platforms_sell = set<pair<int, int>>();
    for (int i = 1; i <= 7; i++) {
        item_demand.insert(make_pair(i, vector<int>()));
        available_demand.insert(make_pair(i, 0));
        item_supply.insert(make_pair(i, vector<int>()));
        item_pending.insert(make_pair(i, vector<int>()));
    }

    // 第二行: 工作台数量
    fgets(line, sizeof line, stdin);
    int platform_count;
    sscanf(line, "%d", &platform_count);

    // 第三行到第三+platform_count行: 工作台信息
    for (int i = 0; i < platform_count; i++) {
        fgets(line, sizeof line, stdin);
        if (line[0] == 'O' && line[1] == 'K')
            return;
        int platform_id, remain_time, material_state, product_state;
        double center_x, center_y;

        // 读入工作台信息
        sscanf(line, "%d %lf %lf %d %d %d", &platform_id, &center_x, &center_y, &remain_time, &material_state, &product_state);

        // 更新工作台信息
        int map_index = platform_map.find(make_pair(center_x, center_y))->second;
        platforms[map_index]->id = platform_id;
        platforms[map_index]->position = make_pair(center_x, center_y);
        platforms[map_index]->remain_time = remain_time;
        platforms[map_index]->material_state = material_state;
        platforms[map_index]->product_state = product_state;

        // 根据工作台原材料格的状态与配方修改产品需求
        if (platform_id >= 4) {
            // 例子，生产配方为110（=6，1+2），当前有1（=2，10），两者相减为100（=4，2），即还需要2号原材料
            int still_need = item_recipes.find(platform_id)->second - material_state;
            for (int j = 1; j <= 7; j++) {
                if (1 << j & still_need) {
                    item_demand[j].push_back(map_index);
                    available_demand[j]++;
                }
            }
        }

        // 判断工作台当前能否提供产品
        if (platforms[map_index]->remain_time == 0 || platforms[map_index]->product_state != 0) {
            item_supply[platform_id].push_back(map_index);
        } else if (platforms[map_index]->remain_time <= 50) {
            item_pending[platform_id].push_back(map_index);
        }
    }

    // 最后四行: 机器人状态, 工作台ID, 物品类型, 时间价值系数, 碰撞价值系数, 角速度, 线速度, 朝向, 坐标
    for (int i = 0; i < 4; i++) {
        fgets(line, sizeof line, stdin);
        if (line[0] == 'O' && line[1] == 'K')
            return;
        int platform_id, item_type;
        double time_value, collision_value, angular_velocity, linear_velocity_x, linear_velocity_y, orientation, position_x, position_y;

        // 读入机器人信息
        sscanf(line, "%d %d %lf %lf %lf %lf %lf %lf %lf %lf", &platform_id, &item_type, &time_value, &collision_value, &angular_velocity, &linear_velocity_x, &linear_velocity_y, &orientation, &position_x, &position_y);

        // 更新机器人信息
        int map_index = i;
        // int map_index = robot_map.find(make_pair(position_x, position_y))->second;
        robots[map_index]->platform_id = platform_id;
        robots[map_index]->item_type = item_type;
        available_demand[item_type]--;// 如果机器人手上有物品，那么需求就会减小，解决自闭问题
        robots[map_index]->time_value = time_value;
        robots[map_index]->collision_value = collision_value;
        if (item_type == 0) {
            robots[map_index]->time_value = 0;
            robots[map_index]->collision_value = 0;
        }
        robots[map_index]->angular_velocity = angular_velocity;
        robots[map_index]->linear_velocity = make_pair(linear_velocity_x, linear_velocity_y);
        robots[map_index]->orientation = orientation;
        robots[map_index]->position = make_pair(position_x, position_y);

        // cerr << "robot " << i << " " << robots[map_index]->position.first << " " << robots[map_index]->position.second << endl;
    }

    // OK行
    fgets(line, sizeof line, stdin);
    if (line[0] == 'O' && line[1] == 'K')
        return;
}

// 初始化物品价格和配方
void initItemPricesRecipes() {
    item_prices.insert(make_pair(1, make_pair(3000, 6000)));
    item_prices.insert(make_pair(2, make_pair(4400, 7600)));
    item_prices.insert(make_pair(3, make_pair(5800, 9200)));
    item_prices.insert(make_pair(4, make_pair(15400, 22500)));
    item_prices.insert(make_pair(5, make_pair(17200, 25000)));
    item_prices.insert(make_pair(6, make_pair(19200, 27500)));
    item_prices.insert(make_pair(7, make_pair(76000, 105000)));

    // 4对应二进制110
    item_recipes.insert(make_pair(4, 6));
    // 5对应二进制1010
    item_recipes.insert(make_pair(5, 10));
    // 6对应二进制1100
    item_recipes.insert(make_pair(6, 12));
    // 7对应二进制1110000
    item_recipes.insert(make_pair(7, 112));
    // 8对应二进制10000000
    item_recipes.insert(make_pair(8, 128));
    // 9对应二进制11111110
    item_recipes.insert(make_pair(9, 254));
}

// f函数
double fFunction(double x, double maxX, double minRate) {
    if (x < maxX)
        return (1 - sqrt(1 - (1 - x / maxX) * (1 - x / maxX))) * (1 - minRate) + minRate;
    else
        return minRate;
}

// 计算时间价值系数
double getTimeValue(int frame_num) {
    return fFunction(frame_num, 9000, 0.8);
}

// 计算碰撞价值系数
double getCollisionValue(double impulse) {
    return fFunction(impulse, 1000, 0.8);
}

// 计算机器人质量
double getRobotMass(int robot_id) {
    if (robots[robot_id]->item_type == 0)
        // 半径0.45m, 密度20kg/m^2
        return radius_without * radius_without * pi * density;
    else
        // 半径0.53m, 密度20kg/m^2
        return radius_with * radius_with * pi * density;
}

// 计算机器人圆心到工作台圆心的距离
double getDistance(double x1, double y1, double x2, double y2) {
    double raw_distance = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)) - distance_within;
    if (raw_distance < ignore_sub)
        return 0;
    return raw_distance;
}

// 排序算法，按照距离从小到大排序，距离相同按照id从小到大排序，输出索引
vector<int> sortDistance(vector<pair<double, int>> &distance_id) {
    sort(distance_id.begin(), distance_id.end(), [](const pair<double, int> &a, const pair<double, int> &b) {
        if (a.first == b.first)
            return a.second < b.second;
        return a.first < b.first;
    });
    vector<int> result;
    for (int i = 0; i < distance_id.size(); i++)
        result.push_back(distance_id[i].second);
    return result;
}

// 判断工作台能否接收物品
bool canSellItem(int robot_id, int platform_id) {
    // 机器人是否有物品
    if (robots[robot_id]->item_type == 0)
        return false;
    // 工作台能否接收物品
    if (item_recipes.find(platforms[platform_id]->id) != item_recipes.end()) {
        // 工作台能接收物品
        // 判断工作台是否已经有该物品
        if (platforms[platform_id]->material_state & (1 << robots[robot_id]->item_type) == 1)
            return false;
        // 判断机器人是否离工作台最近
        if (robots[robot_id]->platform_id == platform_id)
            return true;
        return false;
    }
    return false;
}

// 判断机器人能否购买物品
bool canBuyItem(int frame_id, int robot_id, int platform_id, int money) {
    // 机器人是否有物品
    if (robots[robot_id]->item_type != 0)
        return false;
    // 工作台能否出售物品
    if (platforms[platform_id]->remain_time == 0 || platforms[platform_id]->product_state == 1) {
        // 工作台能出售物品
        // 判断机器人是否离工作台最近
        if (robots[robot_id]->platform_id == platform_id)
            if (money > item_prices.find(platforms[platform_id]->id)->second.first) {
                // 附加判断条件：剩余时间是否足够卖出物品
                vector<pair<double, int>> distance_id_0;
                int pending_item = platforms[platform_id]->id;
                for (int i = 0; i < item_demand[pending_item].size(); i++) {
                    distance_id_0.push_back(make_pair(robots[robot_id]->platform_distance[item_demand[pending_item][i]], item_demand[pending_item][i]));
                }
                robots[robot_id]->platform_distance_sort_sell = sortDistance(distance_id_0);
                if (frame_id + robots[robot_id]->platform_forward_frame[robots[robot_id]->platform_distance_sort_sell[0]] <= max_frames)
                    return true;
                return false;
            }
        return false;
    }
    return false;
}

#endif

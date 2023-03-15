#include "base.h"
#define pi 3.14159265358979323846
#define fps 50
#define radius_without 0.45
#define radius_with 0.53
#define density 20
#define ignore_sub 1e-6

// 牵引力和力矩
#define force 250
#define torque 50

// 可调整参数（好像真的有用）
// 1. 速度过大撞墙问题，修正参数（原始值为1, 1）
#define angle_fix 1
#define forward_fix 1
#define wall_margin 4

// 2. 最大最小前进旋转速度（原始值为6, -2, pi, -pi）
#define max_forward_speed 6
#define min_forward_speed -2
#define max_rotate_speed pi
#define min_rotate_speed -pi

// 3. 机器人是否在工作台判断距离（原始值为0.4）
#define distance_within 0.4

// 4. 贪心策略中，时间系数的估计值
#define time_coefficient 0.9

// 5. 帧数上限，解决购买后时间不足以出售的问题
#define max_frames 8900

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

double total_cost = 0;
double total_sold = 0;

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
    for (int i = 1; i <= 7; i++) {
        item_demand.insert(make_pair(i, vector<int>()));
        available_demand.insert(make_pair(i, 0));
        item_supply.insert(make_pair(i, vector<int>()));
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
        return (1 - sqrt(1 - (x / maxX) * (x / maxX))) * (1 - minRate) + minRate;
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

// 配置机器人到每个工作台的距离
void setRobotPlatformDistanceDirectionTime(int robot_id) {
    // 获取机器人坐标
    double x = robots[robot_id]->position.first;
    double y = robots[robot_id]->position.second;
    // 遍历每个工作台
    for (int i = 0; i < platforms.size(); i++) {
        // 获取工作台坐标
        double x1 = platforms[i]->position.first;
        double y1 = platforms[i]->position.second;

        // 计算方向（需要旋转的角度）
        double current_direction = robots[robot_id]->orientation;     // 范围为-pi到pi
        double target_direction = atan2(y1 - y, x1 - x);              // 范围为-pi到pi
        double delta_direction = target_direction - current_direction;// 正数表示逆时针旋转，负数表示顺时针旋转

        if (abs(delta_direction) < ignore_sub) {
            delta_direction = 0;
        }
        double time_direction = abs(delta_direction / max_rotate_speed);// 最小旋转时间（秒）
        double frame_direction = ceil(time_direction * fps);            // 最小旋转帧数（向上取整）
        time_direction = frame_direction / fps;                         // 最小旋转时间（秒）

        // 更新机器人到工作台的方向的角速度和旋转时间
        if (frame_direction < ignore_sub) {
            // 判断是否撞墙
            if (robots[robot_id]->position.first < wall_margin || robots[robot_id]->position.first > 50 - wall_margin ||
                robots[robot_id]->position.second < wall_margin || robots[robot_id]->position.second > 50 - wall_margin)
                robots[robot_id]->platform_angular_velocity[i] = min_rotate_speed;
            else
                robots[robot_id]->platform_angular_velocity[i] = max(0 - robots[robot_id]->angular_velocity, min_rotate_speed);
            robots[robot_id]->platform_rotate_frame[i] = 0;
        } else {
            robots[robot_id]->platform_angular_velocity[i] = delta_direction / time_direction * angle_fix;
            robots[robot_id]->platform_rotate_frame[i] = frame_direction;
        }

        // 计算距离
        double distance = getDistance(x, y, x1, y1);
        double line_speed = sqrt(robots[robot_id]->linear_velocity.first * robots[robot_id]->linear_velocity.first +
                                 robots[robot_id]->linear_velocity.second * robots[robot_id]->linear_velocity.second);
        robots[robot_id]->platform_distance[i] = distance;
        double time_move = distance / max_forward_speed;// 最小移动时间（秒）
        double frame_move = ceil(time_move * fps);      // 最小移动帧数（向上取整）
        time_move = frame_move / fps;                   // 最小移动时间（秒）

        // 更新机器人到工作台的前进速度和前进时间
        if (frame_move < ignore_sub) {
            if (robots[robot_id]->position.first < wall_margin || robots[robot_id]->position.first > 50 - wall_margin ||
                robots[robot_id]->position.second < wall_margin || robots[robot_id]->position.second > 50 - wall_margin)
                robots[robot_id]->platform_forward_velocity[i] = min_forward_speed;
            else
                robots[robot_id]->platform_forward_velocity[i] = max(0 - line_speed, double(min_forward_speed));
            robots[robot_id]->platform_forward_frame[i] = 0;
        } else {
            robots[robot_id]->platform_forward_velocity[i] = distance / time_move * forward_fix;
            robots[robot_id]->platform_forward_frame[i] = frame_move;
        }
    }
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

// 贪心算法
// 1. 机器人每次找离自己最近的可购买的物品
// 2. 并将该物品出售到离自己最近的（购买后）可接收的平台
// 3. 为了解决最后购买后无法出售的情况，当发现剩余时间无法出售时，不再购买
void greedyAlg2(int frame_id, int money) {
    cout << frame_id << endl;
    cout << flush;

    // 每个机器人分别进行调度
    for (int robot_idx = 0; robot_idx <= 3; robot_idx++) {
        setRobotPlatformDistanceDirectionTime(robot_idx);
        int robot_item = robots[robot_idx]->item_type;
        if (robot_item == 0) {
            // 为0表示需要购买物品
            // 排序策略：按照距离/利润远近排序
            vector<pair<double, int>> distance_id_1;
            // 根据demand需求来分配是否可购买
            for (int item_idx = 1; item_idx <= 7; item_idx++) {
                if (item_demand[item_idx].size() > 0 && available_demand[item_idx] > 0) {
                    for (int i = 0; i < item_supply[item_idx].size(); i++) {
                        double distance_param = robots[robot_idx]->platform_distance[item_supply[item_idx][i]];
                        double loss_param = double(item_prices[item_idx].second) * time_coefficient - double(item_prices[item_idx].first);
                        // distance_id_1.push_back(make_pair(distance_param, item_supply[item_idx][i]));
                        distance_id_1.push_back(make_pair(distance_param / loss_param, item_supply[item_idx][i]));
                    }
                }
            }
            robots[robot_idx]->platform_distance_sort_buy = sortDistance(distance_id_1);

            // 根据距离进行购买
            if (robots[robot_idx]->platform_distance_sort_buy.size() > 0) {
                int fetch_index = robots[robot_idx]->platform_distance_sort_buy[0];
                cout << "rotate " << robot_idx << " " << robots[robot_idx]->platform_angular_velocity[fetch_index] << endl;
                cout << "forward " << robot_idx << " " << robots[robot_idx]->platform_forward_velocity[fetch_index] << endl;
                if (canBuyItem(frame_id, robot_idx, fetch_index, money)) {
                    cout << "buy " << robot_idx << endl;
                    available_demand[platforms[fetch_index]->id]--;
                }
            }
        } else {
            // 需要卖出物品
            vector<pair<double, int>> distance_id_0;
            for (int i = 0; i < item_demand[robot_item].size(); i++) {
                distance_id_0.push_back(make_pair(robots[robot_idx]->platform_distance[item_demand[robot_item][i]], item_demand[robot_item][i]));
            }
            robots[robot_idx]->platform_distance_sort_sell = sortDistance(distance_id_0);

            if (robots[robot_idx]->platform_distance_sort_sell.size() > 0) {
                int fetch_index = robots[robot_idx]->platform_distance_sort_sell[0];
                cout << "rotate " << robot_idx << " " << robots[robot_idx]->platform_angular_velocity[fetch_index] << endl;
                cout << "forward " << robot_idx << " " << robots[robot_idx]->platform_forward_velocity[fetch_index] << endl;
                if (canSellItem(robot_idx, fetch_index)) {
                    cout << "sell " << robot_idx << endl;
                    total_cost += robots[robot_idx]->time_value * robots[robot_idx]->collision_value;
                    total_sold++;
                    // 卖出后判断是否可买入
                    if (canBuyItem(frame_id, robot_idx, fetch_index, money)) {
                        cout << "buy " << robot_idx << endl;
                        available_demand[platforms[fetch_index]->id]--;
                    }
                }
            }
        }

        // 判断是否撞墙（考虑朝向）
        // if ((robots[robot_idx]->position.first < 0.65 && (robots[robot_idx]->orientation >= 0.5 * pi && robots[robot_idx]->orientation <= 1 * pi || robots[robot_idx]->orientation >= -1 * pi && robots[robot_idx]->orientation <= -0.5 * pi)) ||
        //     (robots[robot_idx]->position.first > 49.35 && (robots[robot_idx]->orientation >= 0 * pi && robots[robot_idx]->orientation <= 0.5 * pi || robots[robot_idx]->orientation >= -0.5 * pi && robots[robot_idx]->orientation <= -0 * pi)) ||
        //     (robots[robot_idx]->position.second < 0.65 && (robots[robot_idx]->orientation >= -1 * pi && robots[robot_idx]->orientation <= -0 * pi)) ||
        //     (robots[robot_idx]->position.second > 49.35 && (robots[robot_idx]->orientation >= 0 * pi && robots[robot_idx]->orientation <= 1 * pi))) {
        //     cout << "rotate " << robot_idx << " " << min_rotate_speed << endl;
        //     cout << "forward " << robot_idx << " " << min_forward_speed << endl;
        // }
    }

    cout << "OK" << endl;
    cout << flush;
}

int main() {
    // 读入地图
    initItemPricesRecipes();
    readMapUntilOK();
    cout << "OK" << endl;
    cout << flush;

    int frame_id, money;
    while (scanf("%d %d", &frame_id, &money) != EOF) {
        // 读入每一帧场面信息
        readFrameUntilOK();

        // 贪心算法
        greedyAlg2(frame_id, money);
    }

    // cerr << "total_cost: " << total_cost << endl;
    // cerr << "total_sold: " << total_sold << endl;
    // cerr << "total_coff: " << total_cost / total_sold << endl;
    return 0;
}

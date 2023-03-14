/*
// 贪心算法
// 1. 机器人1-3，每次找离自己最近1-3号物品
// 2. 并送给离自己最近的可接收的平台
void greedyAlg1(int frame_id) {
    cout << frame_id << endl;
    cout << flush;

    setRobotPlatformDistanceDirectionTime(0);
    setRobotPlatformDistanceDirectionTime(1);
    setRobotPlatformDistanceDirectionTime(2);
    setRobotPlatformDistanceDirectionTime(3);

    int schedule[4] = {0, 0, 0, 0};

    // 机器人1-3，每次找离自己最近1-3号物品
    // 排序算法
    for (int r = 1; r <= 3; r++) {
        vector<pair<double, int>> distance_id;
        for (int i = 0; i < item_supply[r].size(); i++)
            distance_id.push_back(make_pair(robots[r]->platform_distance[item_supply[r][i]], item_supply[r][i]));
        robots[r]->platform_distance_sort_buy = sortDistance(distance_id);
    }

    for (int r = 1; r <= 3; r++) {
        vector<pair<double, int>> distance_id;
        for (int i = 0; i < item_demand[r].size(); i++)
            distance_id.push_back(make_pair(robots[r]->platform_distance[item_demand[r][i]], item_demand[r][i]));
        robots[r]->platform_distance_sort_sell = sortDistance(distance_id);
    }

    // 机器人0，每次找离自己最近的4-6号物品
    // 排序算法
    vector<pair<double, int>> distance_id;
    for (int r = 4; r <= 7; r++) {
        for (int i = 0; i < item_supply[r].size(); i++) {
            distance_id.push_back(make_pair(robots[0]->platform_distance[item_supply[r][i]], item_supply[r][i]));
        }
    }
    robots[0]->platform_distance_sort_buy = sortDistance(distance_id);

    distance_id = vector<pair<double, int>>();
    for (int r = 4; r <= 6; r++) {
        for (int i = 0; i < item_demand[r].size(); i++) {
            distance_id.push_back(make_pair(robots[0]->platform_distance[item_demand[r][i]], item_demand[r][i]));
        }
    }
    robots[0]->platform_distance_sort_sell = sortDistance(distance_id);

    // 按照顺序调度物品1-3
    for (int i = 1; i <= 3; i++) {
        if (item_demand[i].size() > 0 && robots[i]->item_type == i) {
            int fetch_index = robots[i]->platform_distance_sort_sell[0];
            cout << "rotate " << i << " " << robots[i]->platform_angular_velocity[fetch_index] << endl;
            cout << "forward " << i << " " << robots[i]->platform_forward_velocity[fetch_index] << endl;
            if (canSellItem(i, fetch_index)) {
                cout << "sell " << i << endl;
                item_demand[i].resize(item_demand.size() - 1);
            }
        } else if (item_supply[i].size() > 0 && robots[i]->item_type == 0) {
            int fetch_index = robots[i]->platform_distance_sort_buy[0];
            cout << "rotate " << i << " " << robots[i]->platform_angular_velocity[fetch_index] << endl;
            cout << "forward " << i << " " << robots[i]->platform_forward_velocity[fetch_index] << endl;
            if (canBuyItem(i, fetch_index)) {
                cout << "buy " << i << endl;
            }
        }
    }

    // 按照顺序调度物品4-6
    int robot_item = robots[0]->item_type;
    if (robot_item == 4 || robot_item == 5 || robot_item == 6) {
        if (robots[0]->platform_distance_sort_sell.size() > 0) {
            int fetch_index = robots[0]->platform_distance_sort_sell[0];
            cout << "rotate " << 0 << " " << robots[0]->platform_angular_velocity[fetch_index] << endl;
            cout << "forward " << 0 << " " << robots[0]->platform_forward_velocity[fetch_index] << endl;
            if (canSellItem(0, fetch_index)) {
                cout << "sell " << 0 << endl;
            }
        }
    } else if (robot_item == 7) {
        distance_id = vector<pair<double, int>>();
        for (int r = 7; r <= 7; r++) {
            for (int i = 0; i < item_demand[r].size(); i++) {
                distance_id.push_back(make_pair(robots[0]->platform_distance[item_demand[r][i]], item_demand[r][i]));
            }
        }
        robots[0]->platform_distance_sort_sell = sortDistance(distance_id);
        if (robots[0]->platform_distance_sort_sell.size() > 0) {
            int fetch_index = robots[0]->platform_distance_sort_sell[0];
            cout << "rotate " << 0 << " " << robots[0]->platform_angular_velocity[fetch_index] << endl;
            cout << "forward " << 0 << " " << robots[0]->platform_forward_velocity[fetch_index] << endl;
            if (canSellItem(0, fetch_index)) {
                cout << "sell " << 0 << endl;
            }
        }
    } else if (robots[0]->platform_distance_sort_buy.size() > 0) {
        int fetch_index = robots[0]->platform_distance_sort_buy[0];
        cout << "rotate " << 0 << " " << robots[0]->platform_angular_velocity[fetch_index] << endl;
        cout << "forward " << 0 << " " << robots[0]->platform_forward_velocity[fetch_index] << endl;
        if (canBuyItem(0, fetch_index)) {
            cout << "buy " << 0 << endl;
        }
    }

    cout << "OK" << endl;
    cout << flush;
}
*/
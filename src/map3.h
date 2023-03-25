#include "base.h"

// 配置机器人到每个工作台的距离
void setRobotPlatformDistanceDirectionTime3(int robot_id) {
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

        // 多转一圈问题修复
        if (delta_direction > pi) {
            delta_direction -= 2 * pi;
        } else if (delta_direction < -pi) {
            delta_direction += 2 * pi;
        }

        // 判断当前角度是否合适
        if (fabs(delta_direction) < ignore_sub) {
            delta_direction = 0;
        }

        // 计算角速度的加速度
        double angular_acceleration = robots[robot_id]->item_type == 0 ? 2.0 * double(torque) / getRobotMass(robot_id) / radius_with / radius_with : 2.0 * double(torque) / getRobotMass(robot_id) / radius_without / radius_without;

        // 从当前角速度减到0的时间
        double time_deceleration_angular_current = fabs(robots[robot_id]->platform_angular_velocity[i] / angular_acceleration);

        // 当前速度减到减速速度所需的距离
        double distance_deceleration_angular_current = 0.5 * fabs(robots[robot_id]->platform_angular_velocity[i]) * time_deceleration_angular_current;

        // 计算当前角加速度
        double time_direction = fabs(delta_direction / max_rotate_speed);// 最小旋转时间（秒）
        double frame_direction = ceil(time_direction * fps);             // 最小旋转帧数（向上取整）
        time_direction = frame_direction / fps;                          // 最小旋转时间（秒）

        // 更新机器人到工作台的方向的角速度和旋转时间
        // 始终开足最大马力
        if (frame_direction < ignore_sub) {
            robots[robot_id]->platform_angular_velocity[i] = 0;
            robots[robot_id]->platform_rotate_frame[i] = 0;
        } else if (fabs(delta_direction) <= distance_deceleration_angular_current + fabs(robots[robot_id]->platform_angular_velocity[i]) * 0.02) {
            robots[robot_id]->platform_angular_velocity[i] = 0;
            robots[robot_id]->platform_rotate_frame[i] = frame_direction;
        } else {
            robots[robot_id]->platform_angular_velocity[i] = delta_direction >= 0 ? max_rotate_speed : min_rotate_speed;
            robots[robot_id]->platform_rotate_frame[i] = frame_direction;
        }

        // 计算距离
        double distance = getDistance(x, y, x1, y1);
        double line_speed = sqrt(robots[robot_id]->linear_velocity.first * robots[robot_id]->linear_velocity.first +
                                 robots[robot_id]->linear_velocity.second * robots[robot_id]->linear_velocity.second);
        robots[robot_id]->platform_distance[i] = distance;

        // 计算转弯半径所需的速度，除了机器人圆心离墙壁的距离，还需要考虑机器人半径
        double current_robot_radius = robots[robot_id]->item_type == 0 ? radius_without : radius_with;
        // 上下左右墙的判断
        double left_wall = robots[robot_id]->position.first - current_robot_radius;
        double right_wall = 50 - robots[robot_id]->position.first - current_robot_radius;
        double up_wall = 50 - robots[robot_id]->position.second - current_robot_radius;
        double down_wall = robots[robot_id]->position.second - current_robot_radius;
        double min_wall_x, min_wall_y;

        if (robots[robot_id]->linear_velocity.first > 0)
            min_wall_x = right_wall;
        else if (robots[robot_id]->linear_velocity.first < 0)
            min_wall_x = left_wall;
        else
            min_wall_x = min(left_wall, right_wall);
        if (robots[robot_id]->linear_velocity.second > 0)
            min_wall_y = up_wall;
        else if (robots[robot_id]->linear_velocity.second < 0)
            min_wall_y = down_wall;
        else
            min_wall_y = min(up_wall, down_wall);

        double min_wall = min(min_wall_x, min_wall_y) * 0.5;

        double decelerate_speed = min(min_wall_x, min_wall_y) < 1.2 ? min(line_speed, fabs(robots[robot_id]->platform_angular_velocity[i]) * min_wall) : max_forward_speed;
        // double decelerate_speed = min(line_speed, fabs(robots[robot_id]->platform_angular_velocity[i]) * min((robots[robot_id]->position.first - current_robot_radius) * 0.5, (robots[robot_id]->position.second - current_robot_radius) * 0.5));

        // 计算线速度的加速度
        double linear_acceleration = double(force) / getRobotMass(robot_id);

        // 从当前速度减到减速速度的时间
        double time_deceleration_current = (line_speed - decelerate_speed) / linear_acceleration;

        // 当前速度减到减速速度所需的距离
        double distance_deceleration_current = decelerate_speed * time_deceleration_current + 0.5 * time_deceleration_current * (line_speed - decelerate_speed);

        // 总时间
        double time_move = distance / line_speed;
        double frame_move = ceil(time_move * fps);// 最小移动帧数（向上取整）
        time_move = frame_move / fps;             // 最小移动时间（秒）

        // 更新机器人到工作台的前进速度和前进时间
        if (frame_move < ignore_sub) {
            robots[robot_id]->platform_forward_velocity[i] = decelerate_speed;
            robots[robot_id]->platform_forward_frame[i] = 0;
        } else if (distance <= distance_deceleration_current + line_speed * 0.02) {
            robots[robot_id]->platform_forward_velocity[i] = decelerate_speed;
            robots[robot_id]->platform_forward_frame[i] = frame_move;
        } else {
            robots[robot_id]->platform_forward_velocity[i] = max_forward_speed;
            robots[robot_id]->platform_forward_frame[i] = frame_move;
        }

        double distance_wall = 0.75 + distance_within;
        if (robots[robot_id]->item_type != 0) {
            if (left_wall < distance_wall && ((robots[robot_id]->orientation > 0.5 * pi && robots[robot_id]->orientation < pi) || (robots[robot_id]->orientation > -pi && robots[robot_id]->orientation < -0.5 * pi)))
                robots[robot_id]->platform_forward_velocity[i] = 0.8;
            if (right_wall < distance_wall && ((robots[robot_id]->orientation > 0 && robots[robot_id]->orientation < 0.5 * pi) || (robots[robot_id]->orientation > -0.5 * pi && robots[robot_id]->orientation < 0)))
                robots[robot_id]->platform_forward_velocity[i] = 0.8;
            if (up_wall < distance_wall && robots[robot_id]->orientation > 0 && robots[robot_id]->orientation < pi)
                robots[robot_id]->platform_forward_velocity[i] = 0.8;
            if (down_wall < distance_wall && robots[robot_id]->orientation > -pi && robots[robot_id]->orientation < 0)
                robots[robot_id]->platform_forward_velocity[i] = 0.8;
        }
    }
}

// 贪心算法
// 1. 机器人每次找离自己最近的可购买的物品
// 2. 并将该物品出售到离自己最近的（购买后）可接收的平台
// 3. 为了解决最后购买后无法出售的情况，当发现剩余时间无法出售时，不再购买
void greedyAlg3(int frame_id, int money) {
    cout << frame_id << endl;
    cout << flush;

    // 每个机器人分别进行调度
    for (int robot_idx = 0; robot_idx <= 3; robot_idx++) {
        setRobotPlatformDistanceDirectionTime3(robot_idx);

        // if(robot_idx != 0){continue;}

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
                        distance_id_1.push_back(make_pair(distance_param / loss_param / available_demand[item_idx], item_supply[item_idx][i]));
                    }
                }

                // 前50帧
                if (frame_id < 50 && item_pending[item_idx].size() > 0 && available_demand[item_idx] > 0) {
                    for (int i = 0; i < item_pending[item_idx].size(); i++) {
                        double distance_param = robots[robot_idx]->platform_distance[item_pending[item_idx][i]];
                        double loss_param = double(item_prices[item_idx].second) * time_coefficient - double(item_prices[item_idx].first);
                        // distance_id_1.push_back(make_pair(distance_param, item_supply[item_idx][i]));
                        distance_id_1.push_back(make_pair(distance_param / loss_param / available_demand[item_idx], item_pending[item_idx][i]));
                    }
                }
            }
            robots[robot_idx]->platform_distance_sort_buy = sortDistance(distance_id_1);

            // 根据距离进行购买
            if (robots[robot_idx]->platform_distance_sort_buy.size() > 0) {
                int fetch_index = -1;
                for (int buy = 0; buy < robots[robot_idx]->platform_distance_sort_buy.size(); buy++) {
                    fetch_index = robots[robot_idx]->platform_distance_sort_buy[buy];
                    // 如果没有匹配到已经选择的平台，则选择
                    if (selected_platforms_buy.find(fetch_index) == selected_platforms_buy.end()) {
                        if (platforms[fetch_index]->remain_time == -1)
                            selected_platforms_buy.insert(fetch_index);
                        break;
                    }
                }

                if (fetch_index == -1)
                    fetch_index = robots[robot_idx]->platform_distance_sort_buy[0];

                cout << "rotate " << robot_idx << " " << robots[robot_idx]->platform_angular_velocity[fetch_index] << endl;
                cout << "forward " << robot_idx << " " << robots[robot_idx]->platform_forward_velocity[fetch_index] << endl;
                if (canBuyItem(frame_id, robot_idx, fetch_index, money)) {
                    cout << "buy " << robot_idx << endl;
                    available_demand[platforms[fetch_index]->id]--;
                    break;
                }
            }
        } else {
            // 需要卖出物品
            vector<pair<double, int>> distance_id_0;
            for (int i = 0; i < item_demand[robot_item].size(); i++) {
                distance_id_0.push_back(make_pair(robots[robot_idx]->platform_distance[item_demand[robot_item][i]], item_demand[robot_item][i]));
            }
            robots[robot_idx]->platform_distance_sort_sell = sortDistance(distance_id_0);

            // 根据距离进行出售
            if (robots[robot_idx]->platform_distance_sort_sell.size() > 0) {
                int fetch_index = -1;
                int nine_index = -1;

                for (int sell = 0; sell < robots[robot_idx]->platform_distance_sort_sell.size(); sell++) {
                    fetch_index = robots[robot_idx]->platform_distance_sort_sell[sell];
                    int can_product = platforms[fetch_index]->product_state == 0 || platforms[fetch_index]->remain_time == -1;
                    // 如果没有匹配到已经选择的平台，则选择
                    if (selected_platforms_sell.find(make_pair(robot_item, fetch_index)) == selected_platforms_sell.end()) {
                        if (platforms[fetch_index]->id == 9 && nine_index == -1) {
                            // 率先匹配到的是9
                            nine_index = fetch_index;
                            fetch_index = -1;
                            continue;
                        }
                        if (platforms[fetch_index]->id != 8 || platforms[fetch_index]->id != 9)
                            selected_platforms_sell.insert(make_pair(robot_item, fetch_index));
                        break;
                    }
                    // 或者放入后可以立即开始生产
                    else if (can_product && ((platforms[fetch_index]->material_state + 1 << robot_item) == item_recipes[robot_item])) {
                        selected_platforms_sell.insert(make_pair(robot_item, fetch_index));
                        break;
                    }
                }

                if (fetch_index == -1 && nine_index != -1)
                    fetch_index = nine_index;
                else if (fetch_index == -1)
                    fetch_index = robots[robot_idx]->platform_distance_sort_sell[0];
                if (platforms[fetch_index]->remain_time == -1)
                    selected_platforms_buy.insert(fetch_index);
                cout << "rotate " << robot_idx << " " << robots[robot_idx]->platform_angular_velocity[fetch_index] << endl;
                cout << "forward " << robot_idx << " " << robots[robot_idx]->platform_forward_velocity[fetch_index] << endl;
                if (canSellItem(robot_idx, fetch_index)) {
                    cout << "sell " << robot_idx << endl;
                    // 卖出后判断是否可买入
                    if (canBuyItem(frame_id, robot_idx, fetch_index, money)) {
                        cout << "buy " << robot_idx << endl;
                        available_demand[platforms[fetch_index]->id]--;
                    }
                }
            }
        }
    }

    // 两个机器人相撞判断
    for (int robot_x = 0; robot_x <= 3; robot_x++) {
        for (int robot_y = 0; robot_y <= 3; robot_y++) {
            if (robot_y == robot_x) {
                continue;
            }
            // 每个机器人的位置
            double robot_x_x = robots[robot_x]->position.first;
            double robot_x_y = robots[robot_x]->position.second;
            double robot_y_x = robots[robot_y]->position.first;
            double robot_y_y = robots[robot_y]->position.second;

            // 每个机器人物品持有情况
            int robot_x_item = robots[robot_x]->item_type;
            int robot_y_item = robots[robot_y]->item_type;
            int item_param = (robot_x_item > 0 ? 1 : 0) + (robot_y_item > 0 ? 1 : 0);

            // 每个机器人的方向
            double robot_x_orientation = robots[robot_x]->orientation;
            double robot_y_orientation = robots[robot_y]->orientation;

            // 两个机器人的距离
            double distance = sqrt(pow(robot_x_x - robot_y_x, 2) + pow(robot_x_y - robot_y_y, 2));

            // 碰撞判定距离
            // double collision_distance = item_param * radius_with + (2 - item_param) * radius_without;
            // double pred_frames = 6 * 0.02;
            // double collision_distance = sqrt(robots[robot_x]->linear_velocity.first * robots[robot_x]->linear_velocity.first + robots[robot_x]->linear_velocity.second * robots[robot_x]->linear_velocity.second) * pred_frames +
            //                             sqrt(robots[robot_y]->linear_velocity.first * robots[robot_y]->linear_velocity.first + robots[robot_y]->linear_velocity.second * robots[robot_y]->linear_velocity.second) * pred_frames;

            // 两个掉头
            double segma = fabs(acos((cos(robot_x_orientation) * (robot_y_x - robot_x_x) + sin(robot_x_orientation) * (robot_y_y - robot_x_y)) / distance));
            if (segma<=pi/2 && distance * cos(segma) < 8 && distance * sin(segma) < 0.9) { // robot_x在自己正前方的感知区域发现了robot_y
                if((robot_y_y - robot_x_y)*robot_y_orientation<0){ // y面朝x而来
                    if(robot_x_x >= robot_y_x){// y从x的左侧来
                        if(robot_y_orientation > 0){
                            cout << "rotate " << robot_x << " " << pi << endl;
                            cout << "rotate " << robot_y << " " << pi << endl;
                        }else{
                            cout << "rotate " << robot_x << " " << -pi << endl;
                            cout << "rotate " << robot_y << " " << -pi << endl;
                        }
                    }else{// y从x的右侧来
                        if(robot_y_orientation > 0){
                            cout << "rotate " << robot_x << " " << -pi << endl;
                            cout << "rotate " << robot_y << " " << -pi << endl;
                        }else{
                            cout << "rotate " << robot_x << " " << pi << endl;
                            cout << "rotate " << robot_y << " " << pi << endl;
                        }
                    }
                }else{ // y拿着东西在x前面跑
                    cout << "forward " << robot_x << " " << 3 << endl;
                }
                break;
            }
        }
    }

    cout << "OK" << endl;
    cout << flush;
}
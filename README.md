# 2023华为软件精英挑战赛-初赛解题算法（C++）

队名：丽岛食堂干净又卫生

成员：[@iamywang](https://github.com/iamywang), [@2022create](https://github.com/2022create), [@HKZ994](https://github.com/HKZ994)

## 贪心算法介绍

### 变量说明

部分全局变量：
- map\<int, pair\<int, int>> item_prices: \<物品编号, \<买入价格, 原始出售价格>>
- map\<int, int> item_recipes: \<工作台编号, 所需物品二进制格表示>
- map\<int, vector\<int>> item_demand: \<物品编号, [当前帧能够接收这种物品的工作台列表]>
- map\<int, int> available_demand: \<物品编号, 当前帧能够接收这种物品的工作台数量-机器人已经买入过该类物品的数量>
- map\<int, vector\<int>> item_supply: \<物品编号, [当前帧机器人能够从哪些工作台购买该物品]>

Robot结构体基本变量：
- int id: 机器人编号
- int platform_id: 机器人所在工作台编号
- int item_type: 机器人持有物品编号
- double time_value: 当前时间损失系数
- double collision_value: 当前碰撞损失系数
- double angular_velocity: 当前旋转角速度
- pair\<double, double> linear_velocity: 当前线速度
- double orientation: 当前方向
- pair\<double, double> position: 当前位置

Robot结构体扩展变量：
- vector\<double> platform_distance: [机器人到每个工作台的直线距离]
- vector\<double> platform_angular_velocity: [机器人到每个工作台的最优角速度]
- vector\<double> platform_rotate_frame: [机器人到每个工作台的最优角速度下的旋转帧数]
- vector\<double> platform_forward_velocity: [机器人到每个工作台的最优线速度]
- vector\<double> platform_forward_frame: [机器人到每个工作台的最优线速度下的移动帧数]
- vector\<int> platform_distance_sort_buy: [机器人到每个工作台的直线距离排序（根据要购买的物品编号）]
- vector\<int> platform_distance_sort_sell: [机器人到每个工作台的直线距离排序（根据要出售的物品编号）]

Platform (Workbench)结构体基本变量：
- int id: 工作台编号
- pair\<double, double> position: 工作台位置
- int remain_time: 工作台剩余生产时间
- int material_state: 工作台原材料状态
- int product_state: 工作台产品状态

### 可调整参数

机器人是否在工作台判断距离（原始值为0.4）：
- #define distance_within 0.4

贪心策略中，时间系数的估计值：
- #define time_coefficient 0.9

帧数上限，解决购买后时间不足以出售的问题：
#define max_frames 8900

### 贪心策略

- 如果机器人目前没有持有物品，则其需要购买物品
    - 根据item_demand和available_demand，机器人可以知道其能够购买哪些种类的物品
    - 根据item_supply，机器人能够知道其能够从哪些工作台购买物品
    - 排序策略1：按照机器人距离工作台的距离（直线距离）进行排序
    - 排序策略2：按照机器人距离工作台的距离（直线距离）/利润（item_prices.second * 损失系数 - item_prices.first）进行排序
    - 选择策略：选择第一个满足条件的工作台，购买后available_demand的值减小1

- 如果机器人目前持有物品，则其需要出售物品
    - 根据item_demand，机器人可以知道其能够向哪些工作台出售该物品
    - 排序策略：按照机器人距离工作台的距离（直线距离）进行排序
    - 选择策略：选择第一个满足条件的工作台，出售后立即判断是否能够买入

- 目前结果
    - 排序策略1：练习赛分数2119593（wall_margin 3.5，距离0.4）
    - 排序策略2：练习赛分数2700343（距离0.39）

### 优化策略

- 损失系数：
    - 时间损失 * 碰撞损失，根据实际情况发现0.9时较优
- 角速度：
    - 按照机器人当前方向和目标方向的夹角，计算出在最大角速度下的旋转时间（精确到帧），并调节角速度
    - 控制机器人时，角速度为该角速度 * 角速度调节系数（1）
- 线速度：
    - 按照机器人当前位置和目标位置的直线距离，计算出在最大线速度下的移动时间（精确到帧），并调节先速度
    - 控制机器人时，线速度为该线速度 * 线速度调节系数（1）
- 角速度与线速度同时调整：
    - 两者同时调整，似乎没发现什么问题（虽然有时可能会有行星旋转的情况）
- 撞墙问题：
    - 调节线速度，切线过弯
- 剩余时间无法满足出售的问题：
    - 设置帧数上限（检测剩余时间无法前进到下一个工作台时，不再购买当前物品）
- 墙角自闭问题：
    - 读入机器人时忘记处理的原因，如果机器人手上有物品，那么需求就会减小

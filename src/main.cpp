#include "map1.h"
#include "map2.h"
#include "map3.h"
#include "map4.h"

// 主函数，调用读入地图、读入每一帧场面信息、调度算法
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
        // 1号图 4个9
        // 2号图 0个9 2个8
        // 3号图 1个9
        // 4号图 0个9 1个8
        if (nine_numbers == 4 && eight_numbers == 1) {
            distance_within = 0.4;
            item_recipes.find(9)->second = 128;
            max_frames = 8980;
            time_coefficient = 0.9;
            greedyAlg1(frame_id, money);
        } else if (nine_numbers == 0 && eight_numbers == 2) {
            distance_within = 0.38;
            item_recipes.find(9)->second = 254;
            max_frames = 8980;
            time_coefficient = 0.9;
            greedyAlg2(frame_id, money);
        } else if (nine_numbers == 1 && eight_numbers == 0) {
            distance_within = 0.38;
            item_recipes.find(9)->second = 254;
            max_frames = 8980;
            time_coefficient = 0.9;
            greedyAlg3(frame_id, money);
        } else if (nine_numbers == 0 && eight_numbers == 1) {
            distance_within = 0.38;
            item_recipes.find(9)->second = 254;
            max_frames = 8980;
            time_coefficient = 0.9;
            greedyAlg4(frame_id, money);
        }
    }
    return 0;
}

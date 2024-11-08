#ifndef GEEKTIME_CHANNEL_MAP_H
#define GEEKTIME_CHANNEL_MAP_H

#include <stdlib.h>
#include <stdbool.h>

/*
 channel_map 虽然取名为 map，其实其内部就是一个数组：
        表面上存储结构为：fd <----> channel*
        事实上，channel* 被存储到了指针数组 entities 中，而与之对应的 fd 就是 channel* 在所在数组中的 index.
 */

/** channel 的映射表，key 为 fd，根据目前的设计，value 的类型为  channel 的指针 */
struct channel_map {
    /** 文件描述符数组 */
    void **entities;
    /** size of entities.*/
    int length;
};

/** 对 channel_map 进行扩容，使其可以至少有 slot 个槽位：
 *
 * <ol>
 * <li>slot 即槽位，每一个 fd 占用一个槽</li>
 * <li>size 为每个 value 需要的空间</li>
 * </ol>
 */
bool channel_map_make_space(struct channel_map *map, int slot, int size);

/** 初始化 channel_map*/
void channel_map_init(struct channel_map *map);

/** 清空 channel_map */
void channel_map_clear(struct channel_map *map);

#endif //GEEKTIME_CHANNEL_MAP_H

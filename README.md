# Trading-System
## 交易系统设计文档
本交易系统实现平均时间复杂度O(1)非新价格挂单,O(logn)新价格挂单,O(1)撤单,O(1)吃单,以及O(1)重整内存
### 数据结构设计如下
- Order & OrderInfo
	- Order类记录每笔订单的id和数量
	- OrderInfo用于记录每个order在vector存储位置的前一个和后一个有效的order的位置（可以类比链表中的 *pPrev 和 *pNext ）
		注：这里说"有效的order"是因为，有些order会被撤单，这样它们就是无效的了，但在vector存储结构中它们还暂时存在，只不过我们通过修改orderInfo的方式把这些无效的order跳过了，在内存重整的时候这些无效的order才会被销毁
- OrderList
	- 维护了以下成员变量
		- vector<Order> __order_list;	//按id从小到大排列的有序order表
		- vector<OrderInfo> __orderInfo_list;	//每一个order的前一个和后一个有效order在存储结构中的位置
		- unordered_map<size_t, size_t> __id_offset_map; //从id映射到这个order在order_list中实际存储位置
		- size_t __first_valid_order_offset;	//没有被撤单的id最小的order在order_list中的存储位置
		- size_t __last_valid_order_offset;		//没有被撤单的id最大的order在order_list中的存储位置
	- 挂单
		- 如果 *__last_valid_order_offset* 后面有空位（也就是之前内存重整过导致__last_valid_order_offset后面也有存储空间），那么就直接在__last_valid_order_offset + 1的位置插入；否则在 *__order_list* 和 *__orderInfo_list* 直接push_back
		- 更新 *__id_offset_map* 让 *__id_offset_map[id]* 指向对应的offset
		- 更新 *__last_valid_order_offset* 指向刚刚push_back的新order的offset
	- 吃单
		由于 *id（timestamp）* 是严格从小到大插入的，所以在 *__order_list* 中存储的order的id一定是从小到大依次递增的，这样保证了在吃单的时候一定是先吃id较小的单，吃单过程如下
		- 通过 *__first_valid_order_offset* 获取第一个有效order的位置（因为有可能前面的单被撤掉，第一个有效单并不在 *__order_list[0]* 的位置）
		- 该单吃掉后，把 *__id_offset_map* 中对应的id号抹掉
		- 通过 *__orderInfo_list* 维护的前后位置关系顺序吃单
		- 吃单操作完成后，更新 *__first_valid_order_offset* （更新头节点位置）和 *__orderInfo_list[__first_valid_order_offset]* （表头指针指向头节点自己）
	- 撤单
		- 通过 *__id_offset_map* 找到该单号的offset，通过 *__orderInfo_list[offset]* 获取这个order的前一个和后一个单的位置，然后更新它俩的前后节点指针（类似于链表删除）
		- 把 *__id_offset_map* 的对应id抹掉
	- 重整内存
		- 就是通过 *__orderInfo_list* 顺序访问每一个有效order，然后把它们挨个按顺序挪到前面，互相挨着
			比如：原来有效订单的排列是 X 1 X 7 X X 11 X , 重整完了变成 1 7 11 X X X X X
- OrderBook
	- 维护了以下成员变量
		- uint16_t __bid_price;	//买方最高价格
		- uint16_t __ask_price;	//卖方最低价格
		- set<uint16_t, greater<uint16_t>> __bid_price_set;	//买方价格序列，用于最优价全部成交之后顺序找到下一个次优价格
		- set<uint16_t, less<uint16_t>> __ask_price_set;	//卖方价格序列，用于最优价全部成交之后顺序找到下一个次优价格
		- unordered_map<uint16_t, OrderList> __price_orderList_map;	//每个价格维护一个orderList
		- unordered_map<size_t, uint16_t> __id_price_map;	//记录每个id的order的价格


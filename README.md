# Trading-System
## 交易系统设计文档
本交易系统实现平均时间复杂度O(1)非新价格挂单,O(logn)新价格挂单,O(1)撤单,O(1)吃单,以及O(1)重整内存
### 数据结构设计如下
- Order & OrderInfo
	- Order类记录每笔订单的id和数量
	- OrderInfo用于记录每个order在vector存储位置的前一个和后一个有效的order的位置（可以类比链表中的 *pPrev 和 *pNext ）
		注：这里说"有效的order"是因为，有些order会被撤单，这样它们就是无效的了，但在vector存储结构中它们还暂时存在，只不过我们通过修改orderInfo的方式把这些无效的order跳过了，在内存重整的时候这些无效的order才会被销毁
- OrderList
![OrderList](/images/OrderList.jpg "OrderList")
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
		- 类似于vector的扩容机制，这个就是反着来的，当orderList中的有效order个数小于 *__last_valid_order_offset* 的1/2时触发
		通过 *__orderInfo_list* 顺序访问每一个有效order，然后把它们挨个按顺序挪到前面，互相挨着
		比如：原来有效订单的排列是 X 1 X 7 X X 11 X , 重整完了变成 1 7 11 X X X X X
- OrderBook
![OrderBook](/images/OrderBook.jpg "OrderBook")
	- 维护了以下成员变量
		- uint16_t __bid_price;	//买方最高价格
		- uint16_t __ask_price;	//卖方最低价格
		- set<uint16_t, greater<uint16_t>> __bid_price_set;	//买方价格序列，用于最优价全部成交之后顺序找到下一个次优价格
		- set<uint16_t, less<uint16_t>> __ask_price_set;	//卖方价格序列，用于最优价全部成交之后顺序找到下一个次优价格
		- unordered_map<uint16_t, OrderList> __price_orderList_map;	//每个价格维护一个orderList
		- unordered_map<size_t, uint16_t> __id_price_map;	//记录每个id的order的价格
	- 每一个价格对应一个OrderList
	- 所有的OrderList按买方序列和卖方序列从小到大排列（红黑树实现）
- 复杂度分析
	- 挂单：
		- 非新价格挂单的时候，可以通过OrderBook的 *__id_price_map* 直接O(1)访问到对应价格的orderList，然后直接在对应的orderList尾部插入即可，复杂度O(1)
		- 新价格挂单的时候，由于要在OrderBook中维护 *__bid_price_set* 或者 *__ask_price_set* ，在它们中插入新的价格，复杂度O(logn)，由于实际场景中新价格挂单很少，或者说新价格挂单往往都是偏离当前市场成交价格很远的地方，logn的挂单复杂度并不会影响性能，因为挂上去了也不会很快成交
	- 吃单：OrderBook分别维护一个买方最高价格和卖方最高价格，吃单时直接通过 *__price_orderList_map* 找到对应的orderList，实现O(1)吃单；注:如果一个价格全部吃完了的话需要顺延到次优价格，这时候这个顺延过程是O(logn)的
	- 撤单：OrderBook撤单时首先通过 *__id_price_map* 实现id到price的映射，也就是直接通过id找到这个order所在的orderList，然后又由于orderList中又有个 *__id_offset_map* 实现从id到offset的映射，两层映射都是O(1)的，所以实现了O(1)撤单
	- 内存重整：类似vector扩容机制，反着来缩容就是内存重整，orderList中有效order的数量小于 *__last_valid_order_offset* 时触发，重整时复杂度O(n)，但由于是每隔2倍才重整一次，所以均摊复杂度O(1)
- 一些可能的优化方案
	- (挂单吃单)和(撤单)这两个操作是互不影响的，可以异步进行
	- 考虑分支预测
	- 行情震荡时，成交价附近的orderList可能会频繁创建和析构，很影响性能。可以参考堆内存的分配机制，一片堆内存即使delete掉了，但操作系统也暂时不给它释放，直到过一段时间没有再次被申请才析构。所以我们可以设置一个机制，当成交价附近的orderList空了的时候也暂时不给它析构，直到市场成交价偏离这个价格比较多的时候再析构
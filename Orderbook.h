#pragma once
#include<unordered_map>
#include<set>

using namespace std;

//订单
class Order {
public:
	size_t __id;			//订单时序编号（timestamp）
	uint16_t __quantity;	//量

	Order& operator=(const Order&);

	Order() = delete;
	Order(const size_t&, const uint16_t&);
};

Order& Order::operator=(const Order& order) {
	__id = order.__id;
	__quantity = order.__quantity;
	return *this;
}

Order::Order(const size_t& id, const uint16_t& quantity) :__id(id), __quantity(quantity) {}

//订单信息
class OrderInfo {
public:
	//bool __invalid;		//该order是否有效
	size_t __previous_valid_offset;	//时序在该order前面的最近有效order的位置
	size_t __next_valid_offset;		//时序在该order后面的最近有效order的位置

	OrderInfo() = delete;
	OrderInfo(const size_t&, const size_t&);
	OrderInfo& operator=(const OrderInfo&);
};

OrderInfo& OrderInfo::operator=(const OrderInfo& order) {
	//__invalid = order.__invalid;
	__previous_valid_offset = order.__previous_valid_offset;
	__next_valid_offset = order.__next_valid_offset;
	return *this;
}

OrderInfo::OrderInfo(const size_t& previous_offset, const size_t& next_offset) :/*__invalid(false),*/ __previous_valid_offset(previous_offset), __next_valid_offset(next_offset) {}

//订单列表，每个价格对应一个OrderList
class OrderList {
public:
	vector<Order> __order_list;	//按id从小到大排列的有序order表
	vector<OrderInfo> __orderInfo_list;	//每一个order对应的信息
	unordered_map<size_t, size_t> __id_offset_map; //从id映射到这个order在order_list中实际存储位置
	size_t __first_valid_order_offset;	//没有被撤单的id最小的order在order_list中的存储位置
	size_t __last_valid_order_offset;	//没有被撤单的id最大的order在order_list中的存储位置

	void add_order(const size_t&, const uint16_t&);	//添加新order
	void cancel_order(const size_t&);	//撤单
	uint16_t take(const uint16_t&);		//成交
	void reallocate_memory();	//当内存超限时重整内存

	OrderList();
	OrderList(const size_t&, const uint16_t&);	//以一个新的order初始化OrderList

	~OrderList();
};

//添加新order
void OrderList::add_order(const size_t& id, const uint16_t& quantity) {
	__order_list.push_back(Order(id, quantity));
	__orderInfo_list.push_back(OrderInfo(__last_valid_order_offset, __orderInfo_list.size()));
	__orderInfo_list[__last_valid_order_offset].__next_valid_offset = __orderInfo_list.size() - 1;
	__last_valid_order_offset = __orderInfo_list.size() - 1;
	__id_offset_map[id] = __last_valid_order_offset;
}

//撤单
void OrderList::cancel_order(const size_t& id) {
	if (__id_offset_map.find(id) == __id_offset_map.end()) return;
	const size_t offset = __id_offset_map[id];
	//if (__orderInfo_list[offset].__invalid) return;
	const size_t previous_offset = __orderInfo_list[offset].__previous_valid_offset;
	const size_t next_offset = __orderInfo_list[offset].__next_valid_offset;
	const uint8_t location = (__orderInfo_list[offset].__previous_valid_offset == __first_valid_order_offset) << 1 | __orderInfo_list[offset].__next_valid_offset == __last_valid_order_offset;
	switch (location) {
		//__orderInfo_list[offset].__invalid = true;
		__id_offset_map.erase(offset);
	case 0:
		//该order位于order_list中间
		__orderInfo_list[previous_offset].__next_valid_offset = next_offset;
		__orderInfo_list[next_offset].__previous_valid_offset = previous_offset;
		break;
	case 1:
		//该order位于order_list尾部
		__orderInfo_list[previous_offset].__next_valid_offset = previous_offset;
		__last_valid_order_offset = previous_offset;
		break;
	case 2:
		//该order位于order_list头部
		__orderInfo_list[next_offset].__previous_valid_offset = next_offset;
		__first_valid_order_offset = next_offset;
		break;
	default:
		//该order是order_list的唯一的订单
		/*
		*	可能的优化：
		*	实际场景中可能出现一个价格频繁挂单又撤单的情况，这样频繁构造和析构OrderList开销比较大
		*	可以考虑使用类似堆内存管理的方式维护，当OrderList为空的时候保持一段时间，发现一段时间内没有新的order插入再析构
		*/
		this->~OrderList();
	}
}

//TODO: 把这个方法转移到OrderBook类中，因为OrderBook中的__id__price_map也需要维护，吃单后被删除的单要再__id_price_map中也移除掉
//吃掉数量为quantity的单，返回吃单方在该价位全部成交后还剩下的未成交的数量
uint16_t OrderList::take(const uint16_t& quantity) {
	uint16_t _quantity = quantity;
	while (_quantity != 0 && !__id_offset_map.empty()) {
		Order& order = __order_list[__first_valid_order_offset];
		if (order.__quantity >= _quantity) {
			//挂单方余量>=吃单方
			order.__quantity -= _quantity;
			_quantity = 0;
		}
		else {
			//挂单方余量<吃单方
			_quantity -= order.__quantity;
			order.__quantity = 0;
		}
		if (order.__quantity == 0) {
			//如果挂单方的order全部成交了
			const size_t offset = __id_offset_map[order.__id];
			__id_offset_map.erase(order.__id);
			__first_valid_order_offset = __orderInfo_list[offset].__next_valid_offset;
			__orderInfo_list[__first_valid_order_offset].__previous_valid_offset = __first_valid_order_offset;
			//__orderInfo_list[offset].__invalid = true;
		}
	}
	return _quantity;
}

//重整内存
void OrderList::reallocate_memory() {
	size_t offset_from = __first_valid_order_offset;
	size_t offset_to = 0;
	__first_valid_order_offset = 0;
	while (true) {
		uint16_t id = __order_list[offset_from].__id;
		__order_list[offset_to] = __order_list[offset_from];
		__orderInfo_list[offset_to] = __orderInfo_list[offset_from];
		if (offset_to == 0) __orderInfo_list[offset_to].__previous_valid_offset = 0;
		else __orderInfo_list[offset_to].__previous_valid_offset = offset_to - 1;
		__orderInfo_list[offset_to].__next_valid_offset = offset_to + 1;
		__id_offset_map[id] = offset_to;
		if (offset_from == __last_valid_order_offset) {
			__orderInfo_list[offset_to].__next_valid_offset = offset_to;
			__last_valid_order_offset = offset_to;
			break;
		}
		offset_from = __orderInfo_list[offset_from].__next_valid_offset;
	}
}

//以一个新的order初始化OrderList
OrderList::OrderList(const size_t& id, const uint16_t& quantity) :__first_valid_order_offset(0), __last_valid_order_offset(0) {
	__order_list.push_back(Order(id, quantity));
	__orderInfo_list.push_back(OrderInfo(0, 0));
	__id_offset_map[id] = 0;
}

//订单簿
class OrderBook {
private:
	uint16_t __bid_price;	//买方最高价格
	uint16_t __ask_price;	//卖方最低价格
	set<uint16_t, greater<uint16_t>> __bid_price_set;	//买方价格序列，用于最优价全部成交之后顺序找到下一个次优价格
	set<uint16_t, less<uint16_t>> __ask_price_set;		//卖方价格序列，用于最优价全部成交之后顺序找到下一个次优价格
	unordered_map<uint16_t, OrderList> __price_orderList_map;	//每个价格维护一个orderList
	unordered_map<size_t, uint16_t> __id_price_map;		//记录每个id的order的价格

	uint16_t take(const uint16_t&, const uint16_t&);	//吃单
public:
	void bid(const size_t&, const uint16_t&, const uint16_t&);	//买单，参数：id, price, quantity
	void ask(const size_t&, const uint16_t&, const uint16_t&);	//卖单，参数：id, price, quantity
	OrderBook();
};

//初始化订单簿
OrderBook::OrderBook() :__bid_price(0), __ask_price(0xffff) {}

//在价格为price处吃单
uint16_t OrderBook::take(const uint16_t& price, const uint16_t& quantity) {
	uint16_t _quantity = quantity;
	OrderList& order_list = __price_orderList_map[price];

}

//买单，参数：id, price, quantity
void OrderBook::bid(const size_t& id, const uint16_t& price, const uint16_t& quantity) {
	if (price < __ask_price) {
		//买单价格小于卖方最低价，挂单
		if (__price_orderList_map.find(price) == __price_orderList_map.end()) {
			//该价格目前没有挂单
			__price_orderList_map[price] = OrderList(id, quantity);
			__bid_price_set.insert(price);
			if (price > __bid_price) {
				//如果该挂单价格高于__bid_price
				__bid_price = price;
				__bid_price_set.insert(price);
			}
		}
		else {
			//该价格目前有挂单
			__price_orderList_map[price].add_order(id, quantity);
		}
		__id_price_map[id] = price;
	}
	else {
		//买单价格大于等于卖方最低价，吃单
		uint16_t quantity_surplus = quantity;
		while (quantity_surplus > 0) {

		}



	}
}

//卖单，参数：id, price, quantity
void OrderBook::ask(const size_t& id, const uint16_t& price, const uint16_t& quantity) {
	if (price > __bid_price) {
		//挂单
	}
	else {
		//吃单
	}
}
#pragma once
#include<iostream>
#include<unordered_map>
#include<set>
#include<iomanip>

#define __ASK false
#define __BID true

typedef bool ACTION;

using namespace std;

//订单
class Order {
public:
	size_t __id;			//订单时序编号（timestamp）
	uint16_t __quantity;	//量

	const Order& operator=(const Order&);

	Order() = delete;
	Order(const size_t&, const uint16_t&);
};

const Order& Order::operator=(const Order& order) {
	if (this == &order) return *this;
	__id = order.__id;
	__quantity = order.__quantity;
	return *this;
}

Order::Order(const size_t& id, const uint16_t& quantity) :__id(id), __quantity(quantity) {}

//订单信息
class OrderInfo {
public:
	size_t __previous_valid_offset;	//时序在该order前面的最近有效order的位置
	size_t __next_valid_offset;		//时序在该order后面的最近有效order的位置

	OrderInfo& operator=(const OrderInfo&);

	OrderInfo() = delete;
	OrderInfo(const size_t&, const size_t&);
};

OrderInfo& OrderInfo::operator=(const OrderInfo& order) {
	if (this == &order) return *this;
	__previous_valid_offset = order.__previous_valid_offset;
	__next_valid_offset = order.__next_valid_offset;
	return *this;
}

OrderInfo::OrderInfo(const size_t& previous_offset, const size_t& next_offset) :__previous_valid_offset(previous_offset), __next_valid_offset(next_offset) {}

//订单列表，每个价格对应一个OrderList
class OrderList {
public:
	vector<Order> __order_list;	//按id从小到大排列的有序order表
	vector<OrderInfo> __orderInfo_list;	//每一个order的前一个和后一个有效order在存储结构中的位置
	unordered_map<size_t, size_t> __id_offset_map; //从id映射到这个order在order_list中实际存储位置
	size_t __first_valid_order_offset;	//没有被撤单的id最小的order在order_list中的存储位置
	size_t __last_valid_order_offset;	//没有被撤单的id最大的order在order_list中的存储位置

	void add_order(const size_t&, const uint16_t&);	//添加新order
	void cancel_order(const size_t&);	//撤单
	void reallocate_memory();	//当内存超限时重整内存
	void print();	//打印

	OrderList& operator=(OrderList&&) noexcept;	//移动赋值运算

	OrderList();
	OrderList(const size_t&, const uint16_t&);	//以一个新的order初始化OrderList

	~OrderList();
};

//打印
void OrderList::print() {
	if (__id_offset_map.empty()) return;
	cout << setw(4) << " ";
	for (auto it = __first_valid_order_offset; it != __last_valid_order_offset; it = __orderInfo_list[it].__next_valid_offset) {
		cout << setw(4) << __order_list[it].__quantity << "(" << __order_list[it].__id << ")";
	}
	cout << setw(4) << __order_list[__last_valid_order_offset].__quantity << "(" << __order_list[__last_valid_order_offset].__id << ")" << endl;
}

//添加新order
void OrderList::add_order(const size_t& id, const uint16_t& quantity) {
	if (__last_valid_order_offset < __order_list.size() - 1) {
		__order_list[__last_valid_order_offset + 1] = Order(id, quantity);
		__orderInfo_list[__last_valid_order_offset + 1] = OrderInfo(__last_valid_order_offset, __last_valid_order_offset + 1);
		__orderInfo_list[__last_valid_order_offset].__next_valid_offset = __last_valid_order_offset + 1;
		__last_valid_order_offset++;
		__id_offset_map[id] = __last_valid_order_offset;
	}
	else {
		__order_list.push_back(Order(id, quantity));
		__orderInfo_list.push_back(OrderInfo(__last_valid_order_offset, __orderInfo_list.size()));
		__orderInfo_list[__last_valid_order_offset].__next_valid_offset = __orderInfo_list.size() - 1;
		__last_valid_order_offset = __orderInfo_list.size() - 1;
		__id_offset_map[id] = __last_valid_order_offset;
	}
	if (__id_offset_map.size() > 2 && __last_valid_order_offset > 2 * __id_offset_map.size()) {
		reallocate_memory();
	}
}

//撤单
void OrderList::cancel_order(const size_t& id) {
	if (__id_offset_map.find(id) == __id_offset_map.end()) return;
	const size_t offset = __id_offset_map[id];
	__id_offset_map.erase(id);
	const size_t previous_offset = __orderInfo_list[offset].__previous_valid_offset;
	const size_t next_offset = __orderInfo_list[offset].__next_valid_offset;
	const uint8_t location = ((offset == __first_valid_order_offset) << 1) | (offset == __last_valid_order_offset);
	switch (location) {
	case 0:
		//该order位于order_list中间
		cout << "该order位于order_list中间" << endl;
		__orderInfo_list[previous_offset].__next_valid_offset = next_offset;
		__orderInfo_list[next_offset].__previous_valid_offset = previous_offset;
		break;
	case 1:
		//该order位于order_list尾部
		cout << "该order位于order_list尾部" << endl;
		__orderInfo_list[previous_offset].__next_valid_offset = previous_offset;
		__last_valid_order_offset = previous_offset;
		break;
	case 2:
		//该order位于order_list头部
		cout << "该order位于order_list头部" << endl;
		__orderInfo_list[next_offset].__previous_valid_offset = next_offset;
		__first_valid_order_offset = next_offset;
		break;
	default:
		//该order是order_list的唯一的订单
		cout << "该order是order_list的唯一的订单" << endl;
		/*
		*	可能的优化：
		*	实际场景中可能出现一个价格频繁挂单又撤单的情况，这样频繁构造和析构OrderList开销比较大
		*	可以考虑使用类似堆内存管理的方式维护，当OrderList为空的时候保持一段时间，发现一段时间内没有新的order插入再析构
		*/
		this->~OrderList();
	}
	if (__id_offset_map.size() > 2 && __last_valid_order_offset > 2 * __id_offset_map.size()) {
		reallocate_memory();
	}
}

//重整内存
void OrderList::reallocate_memory() {
	cout << "重整内存" << endl;
	uint16_t id;
	size_t offset_from = __first_valid_order_offset;
	size_t offset_to = 0;
	while (offset_from!=__last_valid_order_offset) {
		id = __order_list[offset_from].__id;
		__order_list[offset_to] = __order_list[offset_from];
		offset_from = __orderInfo_list[offset_from].__next_valid_offset;
		__orderInfo_list[offset_to].__previous_valid_offset = offset_to - 1;
		__orderInfo_list[offset_to].__next_valid_offset = offset_to + 1;
		__id_offset_map[id] = offset_to;
		offset_to++;
	}
	__orderInfo_list[0].__previous_valid_offset = 0;
	id = __order_list[__last_valid_order_offset].__id;
	__order_list[offset_to] = __order_list[__last_valid_order_offset];
	__orderInfo_list[offset_to].__previous_valid_offset = offset_to - 1;
	__orderInfo_list[offset_to].__next_valid_offset = offset_to;
	__id_offset_map[id] = offset_to;

	__first_valid_order_offset = 0;
	__last_valid_order_offset = offset_to;
}

//移动赋值运算
OrderList& OrderList::operator=(OrderList&& orderList) noexcept {
	if (this == &orderList) return *this;
	__order_list = move(orderList.__order_list);
	__orderInfo_list = move(orderList.__orderInfo_list);
	__id_offset_map = move(orderList.__id_offset_map);
	__first_valid_order_offset = orderList.__first_valid_order_offset;
	__last_valid_order_offset = orderList.__last_valid_order_offset;
	return *this;
}

//以一个新的order初始化OrderList
OrderList::OrderList(const size_t& id, const uint16_t& quantity) :__first_valid_order_offset(0), __last_valid_order_offset(0) {
	__order_list.push_back(Order(id, quantity));
	__orderInfo_list.push_back(OrderInfo(0, 0));
	__id_offset_map[id] = 0;
}

OrderList::OrderList():__first_valid_order_offset(0), __last_valid_order_offset(0) {}

OrderList::~OrderList() {}

//订单簿
class OrderBook {
private:
	uint16_t __bid_price;	//买方最高价格
	uint16_t __ask_price;	//卖方最低价格
	set<uint16_t, greater<uint16_t>> __bid_price_set;	//买方价格序列，用于最优价全部成交之后顺序找到下一个次优价格
	set<uint16_t, less<uint16_t>> __ask_price_set;		//卖方价格序列，用于最优价全部成交之后顺序找到下一个次优价格
	unordered_map<uint16_t, OrderList> __price_orderList_map;	//每个价格维护一个orderList
	unordered_map<size_t, uint16_t> __id_price_map;		//记录每个id的order的价格

	uint16_t take(const uint16_t&, const uint16_t&, const ACTION&);	//吃单
public:
	void bid(const size_t&, const uint16_t&, const uint16_t&);	//买单，参数：id, price, quantity
	void ask(const size_t&, const uint16_t&, const uint16_t&);	//卖单，参数：id, price, quantity
	void cancel(const size_t&);	//撤单
	void print();	//打印orderbook
	OrderBook();
};

//打印orderbook
void OrderBook::print() {
	cout << "====================  Orders  ====================" << endl;
	cout << "Party" << setw(11) << "Price" << setw(16) << "Order List" << endl;
	auto cnt = __ask_price_set.size();
	for (auto it = __ask_price_set.rbegin(); it != __ask_price_set.rend(); it++, cnt--) {
		auto price = *it;
		cout << "Sell " << cnt << setw(10) << price;
		__price_orderList_map[price].print();
	}
	cout << endl;
	cnt = 1;
	for (auto it = __bid_price_set.begin(); it != __bid_price_set.end(); it++, cnt++) {
		auto price = *it;
		cout << "Buy  " << cnt << setw(10) << price;
		__price_orderList_map[price].print();
	}
	cout << "==================================================" << endl << endl;
}

//初始化订单簿
OrderBook::OrderBook() :__bid_price(0), __ask_price(0xffff) {}

//在价格为price处吃单
uint16_t OrderBook::take(const uint16_t& price, const uint16_t& quantity, const ACTION& action) {
	uint16_t _quantity = quantity;
	OrderList& order_list = __price_orderList_map[price];

	while (_quantity != 0 && !order_list.__id_offset_map.empty()) {
		//在当前价格上吃单，直到吃够了quantity或者当前价格上没单可吃了
		Order& order = order_list.__order_list[order_list.__first_valid_order_offset];
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
			const size_t offset = order_list.__id_offset_map[order.__id];
			order_list.__id_offset_map.erase(order.__id);
			order_list.__first_valid_order_offset = order_list.__orderInfo_list[offset].__next_valid_offset;
			order_list.__orderInfo_list[order_list.__first_valid_order_offset].__previous_valid_offset = order_list.__first_valid_order_offset;
			__id_price_map.erase(order.__id);
		}
	}
	if (order_list.__id_offset_map.empty()) {
		//如果这个order_list所对应的价格上面的所有挂单都吃完了
		__price_orderList_map.erase(price);
		if (action == __BID) {
			__ask_price_set.erase(__ask_price);
			if (!__ask_price_set.empty()) __ask_price = *(__ask_price_set.begin());
			else __ask_price = 0xffff;
		}
		else {
			__bid_price_set.erase(__bid_price);
			if (!__bid_price_set.empty()) __bid_price = *(__bid_price_set.begin());
			else __bid_price = 0;
		}
	}
	return _quantity;
}

//买单，参数：id, price, quantity
void OrderBook::bid(const size_t& id, const uint16_t& price, const uint16_t& quantity) {
	if (__id_price_map.find(id) != __id_price_map.end()) return;
	if (quantity == 0) return;
	cout << "bid id:" << id << setw(10) << "price:" << price << setw(15) << "ask price:" << __ask_price << endl;
	if (price < __ask_price) {
		//买单价格小于卖方最低价，挂单
		cout << "买单价格小于卖方最低价，挂单" << endl;
		if (__price_orderList_map.find(price) == __price_orderList_map.end()) {
			//该价格目前没有挂单
			cout << "该价格目前没有挂单" << endl;
			__price_orderList_map[price] = OrderList(id, quantity);
			__bid_price_set.insert(price);
			if (price > __bid_price) {
				//如果该挂单价格高于当前买方最高价，更新
				__bid_price = price;
			}
		}
		else {
			//该价格目前有挂单
			cout << "该价格目前有挂单，直接插入" << endl;
			__price_orderList_map[price].add_order(id, quantity);
		}
		__id_price_map[id] = price;
	}
	else {
		//买单价格大于等于卖方最低价，吃单
		cout << "买单价格大于等于卖方最低价，吃单" << endl;
		uint16_t quantity_surplus = quantity;
		while (quantity_surplus > 0 && price >= __ask_price) {
			quantity_surplus = take(__ask_price, quantity_surplus, __BID);
		}
		if (quantity_surplus > 0) {
			//全都吃完了还有富裕，那么在这个price价格上把剩余没成交的份额挂单
			cout << "全都吃完了还有富裕，那么在这个price价格上把剩余没成交的份额挂单" << endl;
			__id_price_map[id] = price;
			__price_orderList_map[price] = OrderList(id, quantity_surplus);
			__bid_price_set.insert(price);
			__bid_price = price;
		}
	}
}

//卖单，参数：id, price, quantity
void OrderBook::ask(const size_t& id, const uint16_t& price, const uint16_t& quantity) {
	if (__id_price_map.find(id) != __id_price_map.end()) return;
	if (quantity == 0) return;
	cout << "ask id:" << id << setw(10) << "price:" << price << setw(15) << "bid price:" << __bid_price << endl;
	if (price > __bid_price) {
		//卖单价格高于买方最高价，挂单
		cout << "卖单价格高于买方最高价，挂单" << endl;
		if (__price_orderList_map.find(price) == __price_orderList_map.end()) {
			//该价格目前没有挂单
			cout << "该价格目前没有挂单" << endl;
			__price_orderList_map[price] = OrderList(id, quantity);
			__ask_price_set.insert(price);
			if (price < __ask_price) {
				//如果该挂单价格小于当前卖方最低价，更新
				__ask_price = price;
			}
		}
		else {
			//该价格目前有挂单
			cout << "该价格目前有挂单，直接插入" << endl;
			__price_orderList_map[price].add_order(id, quantity);
		}
		__id_price_map[id] = price;

	}
	else {
		//卖单价格低于买方最高价，吃单
		cout << "卖单价格低于买方最高价，吃单" << endl;
		uint16_t quantity_surplus = quantity;
		while (quantity_surplus > 0 && price <= __bid_price) {
			cout << "在" << __bid_price << "处吃" << quantity_surplus << "个单，";
			quantity_surplus = take(__bid_price, quantity_surplus, __ASK);
			cout << "吃完还剩" << quantity_surplus << "个单" << endl;
		}
		if (quantity_surplus > 0) {
			//全都吃完了还有富裕，那么在这个price价格上把剩余没成交的份额挂单
			cout << "全都吃完了还有富裕，那么在这个price价格上把剩余没成交的份额挂单" << endl;
			__id_price_map[id] = price;
			__price_orderList_map[price] = OrderList(id, quantity_surplus);
			__ask_price_set.insert(price);
			__ask_price = price;
		}
	}
}

//撤单
void OrderBook::cancel(const size_t& id) {
	if (__id_price_map.find(id) == __id_price_map.end()) return;
	cout << "撤销订单:" << id << endl;
	const uint16_t price = __id_price_map[id];
	__id_price_map.erase(id);
	if (__price_orderList_map[price].__id_offset_map.size() == 1) {
		//如果这个要撤销的单是该价位下唯一的单
		__price_orderList_map.erase(price);
		if (price == __ask_price) {
			__ask_price_set.erase(price);
			if (!__ask_price_set.empty()) __ask_price = *(__ask_price_set.begin());
			else __ask_price = 0xffff;
		}
		else if (price == __bid_price) {
			__bid_price_set.erase(price);
			if (!__bid_price_set.empty()) __bid_price = *(__bid_price_set.begin());
			else __bid_price = 0;
		}
		else if (price > __ask_price) {
			__ask_price_set.erase(price);
		}
		else {	//price < __bid_price
			__bid_price_set.erase(price);
		}
	}
	else {
		//如果这个要撤销的单不是该价位下唯一的单
		__price_orderList_map[price].cancel_order(id);
		//__price_orderList_map[price].reallocate_memory();
	}
}
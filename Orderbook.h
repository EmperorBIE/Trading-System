#pragma once
#include<iostream>
#include<unordered_map>
#include<set>
#include<iomanip>

#define __ASK false
#define __BID true

typedef bool ACTION;

using namespace std;

//����
class Order {
public:
	size_t __id;			//����ʱ���ţ�timestamp��
	uint16_t __quantity;	//��

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

//������Ϣ
class OrderInfo {
public:
	size_t __previous_valid_offset;	//ʱ���ڸ�orderǰ��������Чorder��λ��
	size_t __next_valid_offset;		//ʱ���ڸ�order����������Чorder��λ��

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

//�����б�ÿ���۸��Ӧһ��OrderList
class OrderList {
public:
	vector<Order> __order_list;	//��id��С�������е�����order��
	vector<OrderInfo> __orderInfo_list;	//ÿһ��order��ǰһ���ͺ�һ����Чorder�ڴ洢�ṹ�е�λ��
	unordered_map<size_t, size_t> __id_offset_map; //��idӳ�䵽���order��order_list��ʵ�ʴ洢λ��
	size_t __first_valid_order_offset;	//û�б�������id��С��order��order_list�еĴ洢λ��
	size_t __last_valid_order_offset;	//û�б�������id����order��order_list�еĴ洢λ��

	void add_order(const size_t&, const uint16_t&);	//�����order
	void cancel_order(const size_t&);	//����
	void reallocate_memory();	//���ڴ泬��ʱ�����ڴ�
	void print();	//��ӡ

	OrderList& operator=(OrderList&&) noexcept;	//�ƶ���ֵ����

	OrderList();
	OrderList(const size_t&, const uint16_t&);	//��һ���µ�order��ʼ��OrderList

	~OrderList();
};

//��ӡ
void OrderList::print() {
	if (__id_offset_map.empty()) return;
	cout << setw(4) << " ";
	for (auto it = __first_valid_order_offset; it != __last_valid_order_offset; it = __orderInfo_list[it].__next_valid_offset) {
		cout << setw(4) << __order_list[it].__quantity << "(" << __order_list[it].__id << ")";
	}
	cout << setw(4) << __order_list[__last_valid_order_offset].__quantity << "(" << __order_list[__last_valid_order_offset].__id << ")" << endl;
}

//�����order
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

//����
void OrderList::cancel_order(const size_t& id) {
	if (__id_offset_map.find(id) == __id_offset_map.end()) return;
	const size_t offset = __id_offset_map[id];
	__id_offset_map.erase(id);
	const size_t previous_offset = __orderInfo_list[offset].__previous_valid_offset;
	const size_t next_offset = __orderInfo_list[offset].__next_valid_offset;
	const uint8_t location = ((offset == __first_valid_order_offset) << 1) | (offset == __last_valid_order_offset);
	switch (location) {
	case 0:
		//��orderλ��order_list�м�
		cout << "��orderλ��order_list�м�" << endl;
		__orderInfo_list[previous_offset].__next_valid_offset = next_offset;
		__orderInfo_list[next_offset].__previous_valid_offset = previous_offset;
		break;
	case 1:
		//��orderλ��order_listβ��
		cout << "��orderλ��order_listβ��" << endl;
		__orderInfo_list[previous_offset].__next_valid_offset = previous_offset;
		__last_valid_order_offset = previous_offset;
		break;
	case 2:
		//��orderλ��order_listͷ��
		cout << "��orderλ��order_listͷ��" << endl;
		__orderInfo_list[next_offset].__previous_valid_offset = next_offset;
		__first_valid_order_offset = next_offset;
		break;
	default:
		//��order��order_list��Ψһ�Ķ���
		cout << "��order��order_list��Ψһ�Ķ���" << endl;
		/*
		*	���ܵ��Ż���
		*	ʵ�ʳ����п��ܳ���һ���۸�Ƶ���ҵ��ֳ��������������Ƶ�����������OrderList�����Ƚϴ�
		*	���Կ���ʹ�����ƶ��ڴ����ķ�ʽά������OrderListΪ�յ�ʱ�򱣳�һ��ʱ�䣬����һ��ʱ����û���µ�order����������
		*/
		this->~OrderList();
	}
	if (__id_offset_map.size() > 2 && __last_valid_order_offset > 2 * __id_offset_map.size()) {
		reallocate_memory();
	}
}

//�����ڴ�
void OrderList::reallocate_memory() {
	cout << "�����ڴ�" << endl;
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

//�ƶ���ֵ����
OrderList& OrderList::operator=(OrderList&& orderList) noexcept {
	if (this == &orderList) return *this;
	__order_list = move(orderList.__order_list);
	__orderInfo_list = move(orderList.__orderInfo_list);
	__id_offset_map = move(orderList.__id_offset_map);
	__first_valid_order_offset = orderList.__first_valid_order_offset;
	__last_valid_order_offset = orderList.__last_valid_order_offset;
	return *this;
}

//��һ���µ�order��ʼ��OrderList
OrderList::OrderList(const size_t& id, const uint16_t& quantity) :__first_valid_order_offset(0), __last_valid_order_offset(0) {
	__order_list.push_back(Order(id, quantity));
	__orderInfo_list.push_back(OrderInfo(0, 0));
	__id_offset_map[id] = 0;
}

OrderList::OrderList():__first_valid_order_offset(0), __last_valid_order_offset(0) {}

OrderList::~OrderList() {}

//������
class OrderBook {
private:
	uint16_t __bid_price;	//����߼۸�
	uint16_t __ask_price;	//������ͼ۸�
	set<uint16_t, greater<uint16_t>> __bid_price_set;	//�򷽼۸����У��������ż�ȫ���ɽ�֮��˳���ҵ���һ�����ż۸�
	set<uint16_t, less<uint16_t>> __ask_price_set;		//�����۸����У��������ż�ȫ���ɽ�֮��˳���ҵ���һ�����ż۸�
	unordered_map<uint16_t, OrderList> __price_orderList_map;	//ÿ���۸�ά��һ��orderList
	unordered_map<size_t, uint16_t> __id_price_map;		//��¼ÿ��id��order�ļ۸�

	uint16_t take(const uint16_t&, const uint16_t&, const ACTION&);	//�Ե�
public:
	void bid(const size_t&, const uint16_t&, const uint16_t&);	//�򵥣�������id, price, quantity
	void ask(const size_t&, const uint16_t&, const uint16_t&);	//������������id, price, quantity
	void cancel(const size_t&);	//����
	void print();	//��ӡorderbook
	OrderBook();
};

//��ӡorderbook
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

//��ʼ��������
OrderBook::OrderBook() :__bid_price(0), __ask_price(0xffff) {}

//�ڼ۸�Ϊprice���Ե�
uint16_t OrderBook::take(const uint16_t& price, const uint16_t& quantity, const ACTION& action) {
	uint16_t _quantity = quantity;
	OrderList& order_list = __price_orderList_map[price];

	while (_quantity != 0 && !order_list.__id_offset_map.empty()) {
		//�ڵ�ǰ�۸��ϳԵ���ֱ���Թ���quantity���ߵ�ǰ�۸���û���ɳ���
		Order& order = order_list.__order_list[order_list.__first_valid_order_offset];
		if (order.__quantity >= _quantity) {
			//�ҵ�������>=�Ե���
			order.__quantity -= _quantity;
			_quantity = 0;
		}
		else {
			//�ҵ�������<�Ե���
			_quantity -= order.__quantity;
			order.__quantity = 0;
		}
		if (order.__quantity == 0) {
			//����ҵ�����orderȫ���ɽ���
			const size_t offset = order_list.__id_offset_map[order.__id];
			order_list.__id_offset_map.erase(order.__id);
			order_list.__first_valid_order_offset = order_list.__orderInfo_list[offset].__next_valid_offset;
			order_list.__orderInfo_list[order_list.__first_valid_order_offset].__previous_valid_offset = order_list.__first_valid_order_offset;
			__id_price_map.erase(order.__id);
		}
	}
	if (order_list.__id_offset_map.empty()) {
		//������order_list����Ӧ�ļ۸���������йҵ���������
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

//�򵥣�������id, price, quantity
void OrderBook::bid(const size_t& id, const uint16_t& price, const uint16_t& quantity) {
	if (__id_price_map.find(id) != __id_price_map.end()) return;
	if (quantity == 0) return;
	cout << "bid id:" << id << setw(10) << "price:" << price << setw(15) << "ask price:" << __ask_price << endl;
	if (price < __ask_price) {
		//�򵥼۸�С��������ͼۣ��ҵ�
		cout << "�򵥼۸�С��������ͼۣ��ҵ�" << endl;
		if (__price_orderList_map.find(price) == __price_orderList_map.end()) {
			//�ü۸�Ŀǰû�йҵ�
			cout << "�ü۸�Ŀǰû�йҵ�" << endl;
			__price_orderList_map[price] = OrderList(id, quantity);
			__bid_price_set.insert(price);
			if (price > __bid_price) {
				//����ùҵ��۸���ڵ�ǰ����߼ۣ�����
				__bid_price = price;
			}
		}
		else {
			//�ü۸�Ŀǰ�йҵ�
			cout << "�ü۸�Ŀǰ�йҵ���ֱ�Ӳ���" << endl;
			__price_orderList_map[price].add_order(id, quantity);
		}
		__id_price_map[id] = price;
	}
	else {
		//�򵥼۸���ڵ���������ͼۣ��Ե�
		cout << "�򵥼۸���ڵ���������ͼۣ��Ե�" << endl;
		uint16_t quantity_surplus = quantity;
		while (quantity_surplus > 0 && price >= __ask_price) {
			quantity_surplus = take(__ask_price, quantity_surplus, __BID);
		}
		if (quantity_surplus > 0) {
			//ȫ�������˻��и�ԣ����ô�����price�۸��ϰ�ʣ��û�ɽ��ķݶ�ҵ�
			cout << "ȫ�������˻��и�ԣ����ô�����price�۸��ϰ�ʣ��û�ɽ��ķݶ�ҵ�" << endl;
			__id_price_map[id] = price;
			__price_orderList_map[price] = OrderList(id, quantity_surplus);
			__bid_price_set.insert(price);
			__bid_price = price;
		}
	}
}

//������������id, price, quantity
void OrderBook::ask(const size_t& id, const uint16_t& price, const uint16_t& quantity) {
	if (__id_price_map.find(id) != __id_price_map.end()) return;
	if (quantity == 0) return;
	cout << "ask id:" << id << setw(10) << "price:" << price << setw(15) << "bid price:" << __bid_price << endl;
	if (price > __bid_price) {
		//�����۸��������߼ۣ��ҵ�
		cout << "�����۸��������߼ۣ��ҵ�" << endl;
		if (__price_orderList_map.find(price) == __price_orderList_map.end()) {
			//�ü۸�Ŀǰû�йҵ�
			cout << "�ü۸�Ŀǰû�йҵ�" << endl;
			__price_orderList_map[price] = OrderList(id, quantity);
			__ask_price_set.insert(price);
			if (price < __ask_price) {
				//����ùҵ��۸�С�ڵ�ǰ������ͼۣ�����
				__ask_price = price;
			}
		}
		else {
			//�ü۸�Ŀǰ�йҵ�
			cout << "�ü۸�Ŀǰ�йҵ���ֱ�Ӳ���" << endl;
			__price_orderList_map[price].add_order(id, quantity);
		}
		__id_price_map[id] = price;

	}
	else {
		//�����۸��������߼ۣ��Ե�
		cout << "�����۸��������߼ۣ��Ե�" << endl;
		uint16_t quantity_surplus = quantity;
		while (quantity_surplus > 0 && price <= __bid_price) {
			cout << "��" << __bid_price << "����" << quantity_surplus << "������";
			quantity_surplus = take(__bid_price, quantity_surplus, __ASK);
			cout << "���껹ʣ" << quantity_surplus << "����" << endl;
		}
		if (quantity_surplus > 0) {
			//ȫ�������˻��и�ԣ����ô�����price�۸��ϰ�ʣ��û�ɽ��ķݶ�ҵ�
			cout << "ȫ�������˻��и�ԣ����ô�����price�۸��ϰ�ʣ��û�ɽ��ķݶ�ҵ�" << endl;
			__id_price_map[id] = price;
			__price_orderList_map[price] = OrderList(id, quantity_surplus);
			__ask_price_set.insert(price);
			__ask_price = price;
		}
	}
}

//����
void OrderBook::cancel(const size_t& id) {
	if (__id_price_map.find(id) == __id_price_map.end()) return;
	cout << "��������:" << id << endl;
	const uint16_t price = __id_price_map[id];
	__id_price_map.erase(id);
	if (__price_orderList_map[price].__id_offset_map.size() == 1) {
		//������Ҫ�����ĵ��Ǹü�λ��Ψһ�ĵ�
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
		//������Ҫ�����ĵ����Ǹü�λ��Ψһ�ĵ�
		__price_orderList_map[price].cancel_order(id);
		//__price_orderList_map[price].reallocate_memory();
	}
}
#pragma once
#include<unordered_map>
#include<set>

using namespace std;

//����
class Order {
public:
	size_t __id;			//����ʱ���ţ�timestamp��
	uint16_t __quantity;	//��

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

//������Ϣ
class OrderInfo {
public:
	//bool __invalid;		//��order�Ƿ���Ч
	size_t __previous_valid_offset;	//ʱ���ڸ�orderǰ��������Чorder��λ��
	size_t __next_valid_offset;		//ʱ���ڸ�order����������Чorder��λ��

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

//�����б�ÿ���۸��Ӧһ��OrderList
class OrderList {
public:
	vector<Order> __order_list;	//��id��С�������е�����order��
	vector<OrderInfo> __orderInfo_list;	//ÿһ��order��Ӧ����Ϣ
	unordered_map<size_t, size_t> __id_offset_map; //��idӳ�䵽���order��order_list��ʵ�ʴ洢λ��
	size_t __first_valid_order_offset;	//û�б�������id��С��order��order_list�еĴ洢λ��
	size_t __last_valid_order_offset;	//û�б�������id����order��order_list�еĴ洢λ��

	void add_order(const size_t&, const uint16_t&);	//�����order
	void cancel_order(const size_t&);	//����
	uint16_t take(const uint16_t&);		//�ɽ�
	void reallocate_memory();	//���ڴ泬��ʱ�����ڴ�

	OrderList();
	OrderList(const size_t&, const uint16_t&);	//��һ���µ�order��ʼ��OrderList

	~OrderList();
};

//�����order
void OrderList::add_order(const size_t& id, const uint16_t& quantity) {
	__order_list.push_back(Order(id, quantity));
	__orderInfo_list.push_back(OrderInfo(__last_valid_order_offset, __orderInfo_list.size()));
	__orderInfo_list[__last_valid_order_offset].__next_valid_offset = __orderInfo_list.size() - 1;
	__last_valid_order_offset = __orderInfo_list.size() - 1;
	__id_offset_map[id] = __last_valid_order_offset;
}

//����
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
		//��orderλ��order_list�м�
		__orderInfo_list[previous_offset].__next_valid_offset = next_offset;
		__orderInfo_list[next_offset].__previous_valid_offset = previous_offset;
		break;
	case 1:
		//��orderλ��order_listβ��
		__orderInfo_list[previous_offset].__next_valid_offset = previous_offset;
		__last_valid_order_offset = previous_offset;
		break;
	case 2:
		//��orderλ��order_listͷ��
		__orderInfo_list[next_offset].__previous_valid_offset = next_offset;
		__first_valid_order_offset = next_offset;
		break;
	default:
		//��order��order_list��Ψһ�Ķ���
		/*
		*	���ܵ��Ż���
		*	ʵ�ʳ����п��ܳ���һ���۸�Ƶ���ҵ��ֳ��������������Ƶ�����������OrderList�����Ƚϴ�
		*	���Կ���ʹ�����ƶ��ڴ����ķ�ʽά������OrderListΪ�յ�ʱ�򱣳�һ��ʱ�䣬����һ��ʱ����û���µ�order����������
		*/
		this->~OrderList();
	}
}

//TODO: ���������ת�Ƶ�OrderBook���У���ΪOrderBook�е�__id__price_mapҲ��Ҫά�����Ե���ɾ���ĵ�Ҫ��__id_price_map��Ҳ�Ƴ���
//�Ե�����Ϊquantity�ĵ������سԵ����ڸü�λȫ���ɽ���ʣ�µ�δ�ɽ�������
uint16_t OrderList::take(const uint16_t& quantity) {
	uint16_t _quantity = quantity;
	while (_quantity != 0 && !__id_offset_map.empty()) {
		Order& order = __order_list[__first_valid_order_offset];
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
			const size_t offset = __id_offset_map[order.__id];
			__id_offset_map.erase(order.__id);
			__first_valid_order_offset = __orderInfo_list[offset].__next_valid_offset;
			__orderInfo_list[__first_valid_order_offset].__previous_valid_offset = __first_valid_order_offset;
			//__orderInfo_list[offset].__invalid = true;
		}
	}
	return _quantity;
}

//�����ڴ�
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

//��һ���µ�order��ʼ��OrderList
OrderList::OrderList(const size_t& id, const uint16_t& quantity) :__first_valid_order_offset(0), __last_valid_order_offset(0) {
	__order_list.push_back(Order(id, quantity));
	__orderInfo_list.push_back(OrderInfo(0, 0));
	__id_offset_map[id] = 0;
}

//������
class OrderBook {
private:
	uint16_t __bid_price;	//����߼۸�
	uint16_t __ask_price;	//������ͼ۸�
	set<uint16_t, greater<uint16_t>> __bid_price_set;	//�򷽼۸����У��������ż�ȫ���ɽ�֮��˳���ҵ���һ�����ż۸�
	set<uint16_t, less<uint16_t>> __ask_price_set;		//�����۸����У��������ż�ȫ���ɽ�֮��˳���ҵ���һ�����ż۸�
	unordered_map<uint16_t, OrderList> __price_orderList_map;	//ÿ���۸�ά��һ��orderList
	unordered_map<size_t, uint16_t> __id_price_map;		//��¼ÿ��id��order�ļ۸�

	uint16_t take(const uint16_t&, const uint16_t&);	//�Ե�
public:
	void bid(const size_t&, const uint16_t&, const uint16_t&);	//�򵥣�������id, price, quantity
	void ask(const size_t&, const uint16_t&, const uint16_t&);	//������������id, price, quantity
	OrderBook();
};

//��ʼ��������
OrderBook::OrderBook() :__bid_price(0), __ask_price(0xffff) {}

//�ڼ۸�Ϊprice���Ե�
uint16_t OrderBook::take(const uint16_t& price, const uint16_t& quantity) {
	uint16_t _quantity = quantity;
	OrderList& order_list = __price_orderList_map[price];

}

//�򵥣�������id, price, quantity
void OrderBook::bid(const size_t& id, const uint16_t& price, const uint16_t& quantity) {
	if (price < __ask_price) {
		//�򵥼۸�С��������ͼۣ��ҵ�
		if (__price_orderList_map.find(price) == __price_orderList_map.end()) {
			//�ü۸�Ŀǰû�йҵ�
			__price_orderList_map[price] = OrderList(id, quantity);
			__bid_price_set.insert(price);
			if (price > __bid_price) {
				//����ùҵ��۸����__bid_price
				__bid_price = price;
				__bid_price_set.insert(price);
			}
		}
		else {
			//�ü۸�Ŀǰ�йҵ�
			__price_orderList_map[price].add_order(id, quantity);
		}
		__id_price_map[id] = price;
	}
	else {
		//�򵥼۸���ڵ���������ͼۣ��Ե�
		uint16_t quantity_surplus = quantity;
		while (quantity_surplus > 0) {

		}



	}
}

//������������id, price, quantity
void OrderBook::ask(const size_t& id, const uint16_t& price, const uint16_t& quantity) {
	if (price > __bid_price) {
		//�ҵ�
	}
	else {
		//�Ե�
	}
}
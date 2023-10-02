#include"Orderbook.h"
#include<string>

int main(void) {
	OrderBook orderBook;
	orderBook.ask(1, 12, 10);
	orderBook.print();
	orderBook.bid(2, 8, 4);
	orderBook.print();
	orderBook.ask(1, 12, 3);
	orderBook.print();
}
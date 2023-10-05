#include"Orderbook.h"
#include<string>

int main(void) {
	OrderBook orderBook;
	orderBook.ask(1, 12, 10);
	orderBook.print();
	orderBook.bid(2, 8, 4);
	orderBook.print();
	orderBook.ask(3, 12, 3);
	orderBook.print();
	orderBook.ask(4, 15, 6);
	orderBook.print();
	orderBook.ask(5, 16, 5);
	orderBook.print();
	orderBook.ask(6, 16, 5);
	orderBook.print();
	orderBook.bid(7, 7, 5);
	orderBook.print();
	orderBook.bid(8, 7, 10);
	orderBook.print();
	orderBook.bid(9, 10, 5);
	orderBook.print();
	orderBook.bid(10, 12, 5);
	orderBook.print();
	orderBook.bid(11, 12, 9);
	orderBook.print();
	orderBook.ask(12, 7, 26);
	orderBook.print();
	orderBook.bid(13, 14, 5);
	orderBook.print();
	orderBook.cancel(6);
	orderBook.print();
	orderBook.bid(14, 15, 3);
	orderBook.print();
	orderBook.ask(15, 15, 3);
	orderBook.print();
	orderBook.cancel(4);
	orderBook.print();
	orderBook.cancel(6);
	orderBook.print();
	orderBook.cancel(5);
	orderBook.print();
	orderBook.cancel(15);
	orderBook.print();
	orderBook.cancel(13);
	orderBook.print();
	orderBook.ask(16, 7, 5);
	orderBook.print();
	orderBook.ask(17, 7, 6);
	orderBook.print();
	orderBook.ask(18, 7, 7);
	orderBook.print();
	orderBook.cancel(18);
	orderBook.print();
}
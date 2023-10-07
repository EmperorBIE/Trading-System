#include"Orderbook.h"
#include<string>

int main(void) {
	OrderBook orderBook;
	orderBook.ask(1, 12, 1);
	orderBook.print();
	orderBook.ask(2, 12, 2);
	orderBook.print();
	orderBook.ask(3, 12, 3);
	orderBook.print();
	orderBook.ask(4, 12, 4);
	orderBook.print();
	orderBook.ask(5, 12, 5);
	orderBook.print();
	orderBook.ask(6, 12, 6);
	orderBook.print();
	orderBook.ask(7, 12, 7);
	orderBook.print();
	orderBook.ask(8, 12, 8);
	orderBook.print();
	orderBook.print();
	orderBook.cancel(3);
	orderBook.print();
	orderBook.cancel(4);
	orderBook.print();
	orderBook.cancel(5);
	orderBook.print();
	orderBook.cancel(6);
	orderBook.print();
	orderBook.cancel(7);
	orderBook.print();
	orderBook.ask(9, 12, 8);
	orderBook.print();
	orderBook.ask(10, 12, 8);

	orderBook.bid(11, 7, 10);
	orderBook.print();
	orderBook.bid(12, 9, 2);
	orderBook.print();
	orderBook.bid(13, 8, 3);
	orderBook.print();
	orderBook.bid(14, 8, 4);
	orderBook.print();
	orderBook.bid(15, 7, 5);
	orderBook.print();
	orderBook.bid(16, 6, 6);
	orderBook.print();
	orderBook.ask(17, 11, 5);
	orderBook.print();
	orderBook.ask(18, 13, 5);
	orderBook.print();
	orderBook.ask(19, 8, 20);
	orderBook.print();

}
# Trading-System
## ����ϵͳ����ĵ�
������ϵͳʵ��ƽ��ʱ�临�Ӷ�O(1)���¼۸�ҵ�,O(logn)�¼۸�ҵ�,O(1)����,O(1)�Ե�,�Լ�O(1)�����ڴ�
### ���ݽṹ�������
- Order & OrderInfo
	- Order���¼ÿ�ʶ�����id������
	- OrderInfo���ڼ�¼ÿ��order��vector�洢λ�õ�ǰһ���ͺ�һ����Ч��order��λ�ã�������������е� *pPrev �� *pNext ��
		ע������˵"��Ч��order"����Ϊ����Щorder�ᱻ�������������Ǿ�����Ч���ˣ�����vector�洢�ṹ�����ǻ���ʱ���ڣ�ֻ��������ͨ���޸�orderInfo�ķ�ʽ����Щ��Ч��order�����ˣ����ڴ�������ʱ����Щ��Ч��order�Żᱻ����
- OrderList
	- ά�������³�Ա����
		- vector<Order> __order_list;	//��id��С�������е�����order��
		- vector<OrderInfo> __orderInfo_list;	//ÿһ��order��ǰһ���ͺ�һ����Чorder�ڴ洢�ṹ�е�λ��
		- unordered_map<size_t, size_t> __id_offset_map; //��idӳ�䵽���order��order_list��ʵ�ʴ洢λ��
		- size_t __first_valid_order_offset;	//û�б�������id��С��order��order_list�еĴ洢λ��
		- size_t __last_valid_order_offset;		//û�б�������id����order��order_list�еĴ洢λ��
	- �ҵ�
		- ��� *__last_valid_order_offset* �����п�λ��Ҳ����֮ǰ�ڴ�����������__last_valid_order_offset����Ҳ�д洢�ռ䣩����ô��ֱ����__last_valid_order_offset + 1��λ�ò��룻������ *__order_list* �� *__orderInfo_list* ֱ��push_back
		- ���� *__id_offset_map* �� *__id_offset_map[id]* ָ���Ӧ��offset
		- ���� *__last_valid_order_offset* ָ��ո�push_back����order��offset
	- �Ե�
		���� *id��timestamp��* ���ϸ��С�������ģ������� *__order_list* �д洢��order��idһ���Ǵ�С�������ε����ģ�������֤���ڳԵ���ʱ��һ�����ȳ�id��С�ĵ����Ե���������
		- ͨ�� *__first_valid_order_offset* ��ȡ��һ����Чorder��λ�ã���Ϊ�п���ǰ��ĵ�����������һ����Ч�������� *__order_list[0]* ��λ�ã�
		- �õ��Ե��󣬰� *__id_offset_map* �ж�Ӧ��id��Ĩ��
		- ͨ�� *__orderInfo_list* ά����ǰ��λ�ù�ϵ˳��Ե�
		- �Ե�������ɺ󣬸��� *__first_valid_order_offset* ������ͷ�ڵ�λ�ã��� *__orderInfo_list[__first_valid_order_offset]* ����ͷָ��ָ��ͷ�ڵ��Լ���
	- ����
		- ͨ�� *__id_offset_map* �ҵ��õ��ŵ�offset��ͨ�� *__orderInfo_list[offset]* ��ȡ���order��ǰһ���ͺ�һ������λ�ã�Ȼ�����������ǰ��ڵ�ָ�루����������ɾ����
		- �� *__id_offset_map* �Ķ�ӦidĨ��
	- �����ڴ�
		- ����ͨ�� *__orderInfo_list* ˳�����ÿһ����Чorder��Ȼ������ǰ�����˳��Ų��ǰ�棬���త��
			���磺ԭ����Ч������������ X 1 X 7 X X 11 X , �������˱�� 1 7 11 X X X X X
- OrderBook
	- ά�������³�Ա����
		- uint16_t __bid_price;	//����߼۸�
		- uint16_t __ask_price;	//������ͼ۸�
		- set<uint16_t, greater<uint16_t>> __bid_price_set;	//�򷽼۸����У��������ż�ȫ���ɽ�֮��˳���ҵ���һ�����ż۸�
		- set<uint16_t, less<uint16_t>> __ask_price_set;	//�����۸����У��������ż�ȫ���ɽ�֮��˳���ҵ���һ�����ż۸�
		- unordered_map<uint16_t, OrderList> __price_orderList_map;	//ÿ���۸�ά��һ��orderList
		- unordered_map<size_t, uint16_t> __id_price_map;	//��¼ÿ��id��order�ļ۸�


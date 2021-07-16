/** 
*  author:  ray'
*  date:    2021/07/15
*  contact: 2582171339@qq.com
*  blog:    https://www.cnblogs.com/rayss
*  github:  https://github.com/XuanRay
*  
*  description: LRU和LFU的实现 
*               LRU code copyed from "小林coding"
*               LFU is coded by ray'
*/

#include <iostream>
#include <list>
#include <unordered_map>
#include <string>

// LRU：adopts 'bidirectional list' and 'hash table' technology
template <typename KeyT, typename ValueT>
class LRUCache {
public:
	typedef std::pair<KeyT, ValueT> Pair;      // using Pair = std::pair<KeyT, ValueT>;
	typedef std::list<Pair> List;
	typedef std::unordered_map<KeyT, typename List::iterator> uMap;

	explicit LRUCache(int cap) : m_capacity(cap) {}

	bool put(KeyT key, ValueT value);          //add element to LRUCache 
	bool get(KeyT key, ValueT* pValue);        //visit element in LRUCache

	std::size_t size() const {
		return m_capacity;
	}

	LRUCache(const LRUCache&) = delete;
	LRUCache& operator=(const LRUCache&) = delete;

	~LRUCache() {
		m_map.clear();
		m_list.clear();
	}

private:
	std::size_t m_capacity;  // list capacity
	List m_list;     // bidirectional list
	uMap m_map;      // hash table
};


template <typename KeyT, typename ValueT>
bool LRUCache<KeyT, ValueT>::put(KeyT key, ValueT value) {
	// find key from hash table
	typename uMap::iterator iter = m_map.find(key);

	// if key is existed
	if (iter != m_map.end()) {
		// delete from bi-list and hash table
		m_list.erase(iter->second);
		m_map.erase(iter);
	}

	// insert the element into bi-list front and hash table
	m_list.push_front(std::make_pair(key, value));
	m_map[key] = m_list.begin();

	// judge whether exceed capacity
	if (m_list.size() > m_capacity) {
		KeyT endKey = m_list.back().first;  // unlike member list::end, which returns an iterator just past this element
											// list::back returns a direct reference
		m_list.pop_back();
		m_map.erase(endKey);
	}

	return true;
}

template <typename KeyT, typename ValueT>
bool LRUCache<KeyT, ValueT>::get(KeyT key, ValueT* pvalue) {
	// find whether key is in hash table
	typename uMap::iterator mapIter = m_map.find(key);
	if (mapIter == m_map.end()) {
		return false;
	}
	
	// get bi-list node
	typename List::iterator listIter = mapIter->second;

	ValueT value = listIter->second;

	// delete node from bi-list and insert into the front.
	m_list.erase(listIter);         //这个迭代器已经失效了，还能push_front? 迭代器失效问题
	m_list.push_front(std::make_pair(key, value));  

	// hash table changed
	m_map[key] = m_list.begin();

	// save the value
	*pvalue = value;

	return true;
}



// FLU adopted double hash table and bidirectional list

// Node information
template <typename KeyT, typename ValueT>
struct Node {
	KeyT key;
	ValueT value;
	int freq;

	Node(KeyT k, ValueT v, int f) : key(k), value(v), freq(f) {}
};

// LFUCache
template<typename KeyT, typename ValueT>
class LFUCache {
public:
	typedef std::unordered_map<int, std::list<Node<KeyT, ValueT>>> Freq_Table;
	typedef std::unordered_map<KeyT, typename std::list<Node<KeyT, ValueT>>::iterator> Key_Table;

	// ctor
	LFUCache(int cap = 1) : m_minfreq(0), m_capacity(cap) {}

	// put and get
	bool get(KeyT key, ValueT* pValue);
	bool put(KeyT key, ValueT value);

	std::size_t get_MinFreq() const {
		return this->m_minfreq;
	}

	std::size_t get_Capacity() const {
		return this->m_capacity;
	}

	// dtor
	~LFUCache() {
		m_freqTable.clear();
		m_keyTable.clear();
	}

private:
	std::size_t m_minfreq;
	std::size_t m_capacity;
	Freq_Table m_freqTable;
	Key_Table m_keyTable;
};

template<typename KeyT, typename ValueT>
bool LFUCache<KeyT, ValueT>::get(KeyT key, ValueT* pValue) {
	// find whether key is in key hash table
	auto iter = m_keyTable.find(key);
	if (iter == m_keyTable.end()) {
		return false;
	}

	typename std::list<Node<KeyT, ValueT>>::iterator node = iter->second;
	*pValue = node->value;
	int freq = node->freq;

	// delete this node
	m_freqTable[freq].erase(node);

	// if current list is NULL, delete it from freq hash table
	if (m_freqTable[freq].size() == 0) {
		m_freqTable.erase(freq);
		if (m_minfreq == freq) {
			m_minfreq += 1;
		}
	}

	// insert the node to freq + 1 list
	m_freqTable[freq + 1].push_front(Node<KeyT, ValueT>(key, *pValue, freq + 1));

	//m_freqTable[freq + 1].push_front(new Node<KeyT, ValueT>(key, val, freq + 1));
	m_keyTable[key] = m_freqTable[freq + 1].begin();
	
	return true;
}

template<typename KeyT, typename ValueT>
bool LFUCache<KeyT, ValueT>::put(KeyT key, ValueT value) {
	typename Key_Table::iterator iter = this->m_keyTable.find(key);

	// if key is not found in key hash table
	if (iter == m_keyTable.end()) {
		// if full
		if (m_capacity == m_keyTable.size()) {
			auto it = m_freqTable[m_minfreq].back(); // list的back()返回的是reference
			m_keyTable.erase(it.key);
			m_freqTable[m_minfreq].pop_back();
			if (m_freqTable[m_minfreq].size() == 0) {
				m_freqTable.erase(m_minfreq);
			}
		}
		m_freqTable[1].push_front(Node<KeyT, ValueT>(key, value, 1));
		m_keyTable[key] = m_freqTable[1].begin();
		m_minfreq = 1;
	}
	else { // otherwise
		auto node = iter->second;
		int freq = node->freq;
		m_freqTable[freq].erase(node);
		if (m_freqTable[freq].size() == 0) {
			m_freqTable.erase(freq);
			if (m_minfreq == freq) {
				m_minfreq += 1;
			}
		}
		m_freqTable[freq + 1].push_front(Node<KeyT, ValueT>(key, value, freq + 1));
		m_keyTable[key] = m_freqTable[freq + 1].begin();
	}

	return true;
}



// test LRU
void testLRU() {
	LRUCache<int, std::string> lruCache(3);

	lruCache.put(1, "r");
	lruCache.put(2, "a");
	lruCache.put(3, "y");

	std::string value;
	bool ret = true;

	ret = lruCache.get(1, &value);
	std::cout << "value = " << value << ", ret = " << ret << "\n";

	lruCache.put(4, "'");
	value = "";
	ret = lruCache.get(2, &value);
	std::cout << "value = " << value << ", ret = " << ret << "\n";
}

void testLFU() {
	LFUCache<int, std::string> lfuCache(3);
	lfuCache.put(1, "r");
	lfuCache.put(2, "a");
	lfuCache.put(3, "y");

	std::string value;
	bool ret = true;

	ret = lfuCache.get(1, &value);
	std::cout << "value = " << value << ", ret = " << ret << "\n";

	value = "";
	ret = lfuCache.get(2, &value);
	std::cout << "value = " << value << ", ret = " << ret << "\n";

	lfuCache.put(4, "s");

	value = "";
	ret = lfuCache.get(3, &value);
	std::cout << "value = " << value << ", ret = " << ret << "\n";
}


// test LRU and LFU
int main() {
	testLRU();

	std::cout << "\n\n";
	std::cout << "----------------------------------------------\n\n";

	testLFU();

	system("pause");
	return 0;
}
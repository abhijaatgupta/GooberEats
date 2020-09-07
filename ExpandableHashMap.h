// ExpandableHashMap.h
#include <list>

template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
	ExpandableHashMap(double maximumLoadFactor = 0.5);
    ~ExpandableHashMap();// destructor; deletes all of the items in the hashmap
    void reset(); // resets the hashmap back to 8 buckets, deletes all items
    int size() const; // return the number of associations in the hashmap
    // The associate method associates one item (key) with another (value).
    // If no association currently exists with that key, this method inserts
    // a new association into the hashmap with that key/value pair. If there is // already an association with that key in the hashmap, then the item
    // associated with that key is replaced by the second parameter (value).
    // Thus, the hashmap must contain no duplicate keys.
    void associate(const KeyType& key, const ValueType& value);
    // If no association exists with the given key, return nullptr; otherwise,
    // return a pointer to the value associated with that key. This pointer can be
    // used to examine that value, and if the hashmap is allowed to be modified, to
    // modify that value directly within the map (the second overload enables
    // this). Using a little C++ magic, we have implemented it in terms of the
    // first overload, which you must implement.
    const ValueType* find(const KeyType& key) const;

	  // for a modifiable map, return a pointer to modifiable ValueType
	ValueType* find(const KeyType& key)
	{
		return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
	}
	ExpandableHashMap(const ExpandableHashMap&) = delete;
	ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;

private:
    double loadFactor;
    struct Node{
        KeyType m_key;
        ValueType m_value;
        Node(KeyType key, ValueType value){
            m_key = key;
            m_value = value;
        }
    };
    int m_size;
    int m_length;
    std::list<Node>* keyMap;
    
    unsigned int getNodeValue(const KeyType& key) const {
        unsigned int hasher(const KeyType& k); // prototype
        unsigned int hashed = hasher(key);
        return hashed % m_length;
    }
};

template <typename KeyType, typename ValueType> ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double maximumLoadFactor)
{
    loadFactor = maximumLoadFactor;
    if (loadFactor <= 0)
        loadFactor = 0.5;
    keyMap = new std::list<Node>[8];
    m_size = 0;
    m_length = 8;
}

template <typename KeyType, typename ValueType> ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap()
{
    delete [] keyMap;
}

template <typename KeyType, typename ValueType> void ExpandableHashMap<KeyType, ValueType>::reset()
{
    delete [] keyMap;
    keyMap = new std::list<Node>[8]; //create new array of size 8 to replace deleted one
    m_size = 0;
    m_length = 8;
}

template <typename KeyType, typename ValueType> int ExpandableHashMap<KeyType, ValueType>::size() const
{
    return m_size;//return size of array which is stored as variable and constantly updated
}

template <typename KeyType, typename ValueType> void ExpandableHashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value)
{
    ValueType* val = find(key);
    if (val == nullptr){
        m_size++;
        if ((double)m_size/m_length > loadFactor){
            //make new map with double the size
            m_length = m_length * 2;
            std::list<Node>* newList = keyMap;
            keyMap = new std::list<Node>[m_length];
            for (int i = 0; i < m_length/2; i++){ //for every value in the old map
                typename std::list<Node>::iterator p;
                p = newList[i].begin();
                while(p!= newList[i].end()){
                    Node newEntry(p->m_key, p->m_value);
                    keyMap[getNodeValue(p->m_key)].push_back(newEntry); //keep rehashing the values that were previously stored in the map
                    p++;
                }
            }
            //delete old keyMap
            delete [] newList;
        }
        //insert key val pair to hashmap
        int n = getNodeValue(key);
        Node newEntry(key, value);
        keyMap[n].push_back(newEntry);
    }
    else {
        *val = value;
    }
}

template <typename KeyType, typename ValueType> const ValueType* ExpandableHashMap<KeyType, ValueType>::find(const KeyType& key) const
{
    int i = getNodeValue(key); //unhash the key to get the list in the array where the value would be stored if in Hashmap
    typename std::list<Node>::iterator p;
    p = keyMap[i].begin();
    while(p != keyMap[i].end()){ //until the end of the list we found earlier
        if(p->m_key == key){ //case for a key matching the key we are looking for
            return &(p->m_value);
        }
        p++;
    }
    return nullptr;
}

#ifndef     __RADIX_RADIXTREE_H__
# define    __RADIX_RADIXTREE_H__

# include   <memory>
# include   <map>
# include   <utility>
# include   <set>
# include   <iostream>
# include   <mutex>

# include   <cstdio>

class RadixTree
{
  public :

    inline explicit  RadixTree():
            _trunk(std::make_shared< s_leaf >()),
            _garbage(std::make_unique<garbage>()),
            _dico(std::make_unique<dico>()),
            _nbEntry(0),
            _nbLeaf(0),
            _mutex()
    {}

    inline ~RadixTree() {
      for (const auto & oneLeaf : *_garbage)
        oneLeaf->childLeaf->clear();
    }

    /** Structure */
    struct s_leaf
    {

      typedef std::map< char, std::shared_ptr< struct s_leaf > > branch;

      std::string               key;       // piece present in the tree
      std::string               indexedKey;// total key if leaf is an end | cqn be kick
      const void *              storedVal; // val if leaf is an end
      struct s_leaf *           rootLeaf;  // parent leaf
      std::shared_ptr< branch > childLeaf; // child leaf
# ifdef DEBUG_RADIX
      size_t                    deepness;  // deepness in the tree | start of key if indexedKey exist
      size_t                    id;        // unique id of the leaf
# endif // !DEBUG_RADIX
      std::mutex                mutex;

      explicit s_leaf(std::string key = "",
                      std::string indexedKey = "",
                      const void * storedVal = nullptr
# ifdef DEBUG_RADIX
                      ,size_t      pos = 0,
                      size_t      id = 0
# endif // !DEBUG_RADIX
                     ) :
              key(std::move(key)),
              indexedKey(std::move(indexedKey)),
              storedVal(storedVal),
              rootLeaf(nullptr),
              childLeaf(std::make_shared< branch >())
# ifdef DEBUG_RADIX
              ,deepness(pos),
              id(id)
# endif // 1DEBUG_RADIX
      {}

      friend std::ostream & operator<<(std::ostream & os, const typename RadixTree::s_leaf & leaf)
      {
# ifdef DEBUG_RADIX
        os << "----------[LeafID]:[" << leaf.id << "]----------\n["
           << leaf.key << "]/[" << leaf.indexedKey << "] = [" << &leaf.storedVal
           << "]\n[RootID]:\t"
           << ( leaf.rootLeaf ? leaf.rootLeaf->id : 0 ) << "\n[ChildIDs]:\t"
                ;
        for (const auto & oneLeaf : *leaf.childLeaf)
          os << oneLeaf.second->id << " ";
# else
        os << "----------[Leaf]:[" << leaf.key << "]----------\n["
           << leaf.indexedKey << "] = [" << leaf.storedVal
           << "]\n[RootKey]:\t"
           << ( leaf.rootLeaf ? leaf.rootLeaf->key : "" ) << "\n[ChildKeys]:\t"
                ;
        for (const auto & oneLeaf : *leaf.childLeaf)
          os << oneLeaf.second->key << " ";
# endif // !DEBUG_RADIX

        return os;
      }

    };

    /** Public Methods */
    inline bool loadString(const std::string & key, const void * val)
    {
//      if (!std::is_same<StoredType, std::shared_ptr>) {}
      return deepestInsert(key, val);
    }

    inline void    print() const
    {
      std::cout << "There is " << this->_nbEntry << " entries for " << this->_nbLeaf << " leafs.\n" << std::endl;
      for (const auto & oneLeaf : *this->_garbage)
        std::cout << *oneLeaf << std::endl;
      std::cout << "Lil resume" << std::endl;
      for (const auto & oneEntry : *this->_dico)
        std::cout << oneEntry.first << ":" << oneEntry.second << std::endl;
    }

    inline const std::shared_ptr< s_leaf > longestPrefixMatch(const std::string & key) const
    {
      auto   itLeaf = this->_trunk;
      size_t i      = 0;

      while (!itLeaf->childLeaf->empty()) {

        auto tmpLeaf = itLeaf->childLeaf->find(key[i]);
        if (tmpLeaf != itLeaf->childLeaf->end()) {
          // advance iterator & strings
          itLeaf           = tmpLeaf->second;
          size_t itLeafKey = 0;
          for (; key[i] && itLeaf->key[itLeafKey] && key[i] == itLeaf->key[itLeafKey]; i++, itLeafKey++);

          if (( !key[i] && !itLeaf->key[itLeafKey] )   // perfect match
              || ( key[i] && itLeaf->key[itLeafKey] )) // partial match (lpm)
            return itLeaf;
        } else // we are in the middle of our tree with no match
          return itLeaf;
      }

      return itLeaf;
    }

    inline const std::pair<std::string, const void *> perfectMatch(const std::string & key) const
    {
      auto retVal = this->_dico->find(key);
      return (retVal == this->_dico->end()) ? std::make_pair(key, nullptr) : std::make_pair(key, retVal->second);
    }

  private :

    /** Typedef */
    typedef std::set< std::shared_ptr< s_leaf > >       garbage;
    typedef std::map< const std::string, const void * > dico;

    /** Methods */
    inline bool addLeaf(
            struct s_leaf * leaf,    // append to this leaf
            const std::string & key, // this new key
            const void * val,  // with this value
            size_t pos)              // where to cut the key
    {
      std::string partialKey = key.c_str() + pos;
      char index             = partialKey[0];
      if (pos)
        leaf->mutex.lock();
      auto mapPair           = leaf->childLeaf->emplace(
          index, std::make_shared< struct s_leaf >(partialKey, key, val
# ifdef DEBUG_RADIX
                      , pos, _nbLeaf + 1
# endif // !DEBUG_RADIX

                                                  ));
      auto newLeaf           = mapPair.first->second;

      if (!mapPair.second) {// if emplace failed
        if (pos)
          leaf->mutex.unlock();
        return false;
      }

      this->_mutex.lock();
      _garbage->insert(newLeaf);
      _dico->insert(std::pair<std::string, const void *>(key, val));
      _nbEntry++;
      _nbLeaf++;
      this->_mutex.unlock();
      if (pos)
        newLeaf->mutex.lock();
      newLeaf->rootLeaf = leaf;
      newLeaf->mutex.unlock();

      leaf->mutex.unlock();
# ifdef DEBUG_RADIX
      printf("Added a new leaf (%lu) to the tree.\n", newLeaf->id);
# else
      printf("Added a new leaf (%s) to the tree.\n", newLeaf->key.c_str());
# endif // !DEBUG_RADIX

      return true;
    }

    inline bool deepestInsert(const std::string & key, const void * val)
    {
      size_t        i      = 0;
      auto          itLeaf = _trunk;

      while (!itLeaf->childLeaf->empty()) {

        auto tmpLeaf = itLeaf->childLeaf->find(key[i]);
        if (tmpLeaf != itLeaf->childLeaf->end()) {
          // advance iterator & strings
          itLeaf           = tmpLeaf->second;
          size_t itLeafKey = 0;
          for (; key[i] && itLeaf->key[itLeafKey] && key[i] == itLeaf->key[itLeafKey]; i++, itLeafKey++);

          // entry in tree but not linked to value ?
          if (!key[i] && !itLeaf->key[itLeafKey]) {
            if (itLeaf->storedVal != nullptr) { printf("Entry key/val already present. Kick // maybe update ?\n");
              return false;
            }
            printf("Leaf is there but not the val.\n");
            itLeaf->storedVal = val;
            this->_mutex.lock();
            _dico->insert(std::pair<std::string, const void *>(key, val));
            _nbEntry++;
            this->_mutex.unlock();
            return true;
          }
          else if (key[i] && itLeaf->key[itLeafKey])
            return splitNode(itLeaf, key, val, i, itLeafKey);
        } else // we are in the middle of our tree with no match
          return addLeaf(&*itLeaf, key, val, i);
      }

      // we are on a end node without matching child
# ifdef DEBUG_RADIX
      printf("Append a new leaf to the leaf id (%lu).\n", itLeaf->id);
# else
      printf("Append a new leaf to the leaf id (%s).\n", itLeaf->key.c_str());
# endif // !DEBUG_RADIX
      addLeaf(&*itLeaf, key, val, i);

      return false;
    }

    inline bool splitNode(
            std::shared_ptr< struct s_leaf > leaf, // split this leaf
            const std::string & key,             // to add this key
            const void * val,             // with this val
            size_t deepness,                     // where to cut key
            size_t cutLeafAt)                    // where to split the leaf
    {
      std::string token;
      leaf->mutex.lock();
      s_leaf *    rootLeaf  = leaf->rootLeaf;
      rootLeaf->mutex.lock();
       size_t      i         = 0;

      // define partial string
      for (; i < cutLeafAt; i++)
        token += leaf->key[i];

      // create & link the new leaf
      rootLeaf->childLeaf->erase(token[0]);
      auto mapPair           = rootLeaf->childLeaf->emplace(
              token[0], std::make_shared< struct s_leaf >(token, "", nullptr
# ifdef DEBUG_RADIX
                      , leaf->deepness, _nbLeaf + 1
# endif // !DEBUG_RADIX
                                                         ));
      auto newLeaf           = mapPair.first->second;
      newLeaf->mutex.lock();
      newLeaf->rootLeaf = rootLeaf;

      this->_mutex.lock();
      _garbage->insert(newLeaf);
      _nbLeaf++;
      this->_mutex.unlock();

      // define 2nd token
      token.clear();
      for (; leaf->key[i]; i++)
        token += leaf->key[i];

      // update & link the old leaf
# ifdef DEBUG_RADIX
      leaf->deepness = deepness;
# endif
      leaf->key      = token;
      newLeaf->childLeaf->emplace(token[0], leaf);
      leaf->rootLeaf = &*newLeaf;

      leaf->mutex.unlock();
      newLeaf->mutex.unlock();

# ifdef DEBUG_RADIX
      printf("Added a half leaf (%lu) / updated some old leaf (%lu and %lu).\n", newLeaf->id, leaf->id, rootLeaf->id);
# else
      printf("Added a half leaf (%s) / updated some old leaf (%s and %s).\n", newLeaf->key.c_str(), leaf->key.c_str(), rootLeaf->key.c_str());
# endif // !DEBUG_RADIX
      return addLeaf(&*newLeaf, key, val, deepness);
    }

    /** Attribute */
    std::shared_ptr< s_leaf >               _trunk;
    std::unique_ptr< garbage >              _garbage;
    std::unique_ptr< dico >                 _dico;
    size_t                                  _nbEntry;
    size_t                                  _nbLeaf;
    std::mutex                              _mutex;
};

# endif //!__RADIX_RADIXTREE_H__

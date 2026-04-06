#pragma once

namespace RBX::Reflection
{
	template<typename Key>
	struct DenseHashSetItem
	{
		Key key;

		DenseHashSetItem(const Key& key) :
		    key(key)
		{
		}
	};

	template<typename Key, typename Value>
	struct DenseHashMapItem
	{
		Key key;
		Value value;

		DenseHashMapItem(const Key& key) :
		    key(key),
		    value()
		{
		}
	};

	template<typename Key, typename Item, typename Hash, typename Eq>
	class DenseHashTable
	{
	public:
		class const_iterator;

		DenseHashTable(const Key& empty_key, size_t buckets = 0) :
		    data(buckets, Item(empty_key)),
		    count(0),
		    empty_key(empty_key)
		{
		}

		void clear()
		{
			data.clear();
			count = 0;
		}

		Item* insert(const Key& key)
		{
			if (count >= data.size() * 3 / 4)
			{
				rehash();
			}

			size_t hashmod = data.size() - 1;
			size_t bucket  = hasher(key) & hashmod;

			for (size_t probe = 0; probe <= hashmod; ++probe)
			{
				Item& probe_item = data[bucket];

				if (eq(probe_item.key, empty_key))
				{
					probe_item.key = key;
					count++;
					return &probe_item;
				}

				if (eq(probe_item.key, key))
				{
					return &probe_item;
				}

				bucket = bucket + probe + 1 & hashmod;
			}

			return nullptr;
		}

		const Item* find(const Key& key) const
		{
			if (data.empty())
				return nullptr;
			if (eq(key, empty_key))
				return nullptr;

			size_t hashmod = data.size() - 1;
			size_t bucket  = hasher(key) & hashmod;

			for (size_t probe = 0; probe <= hashmod; ++probe)
			{
				const Item& probe_item = data[bucket];

				if (eq(probe_item.key, key))
					return &probe_item;
				if (eq(probe_item.key, empty_key))
					return nullptr;

				bucket = bucket + probe + 1 & hashmod;
			}

			return nullptr;
		}

		const_iterator begin() const
		{
			size_t start = 0;

			while (start < data.size() && eq(data[start].key, empty_key))
				start++;

			return const_iterator(this, start);
		}

		const_iterator end() const
		{
			return const_iterator(this, data.size());
		}

		size_t size() const
		{
			return count;
		}

		size_t bucket_count() const
		{
			return data.size();
		}

		class const_iterator
		{
		public:
			const_iterator() :
			    set(nullptr),
			    index(0)
			{
			}

			const_iterator(const DenseHashTable* set, const size_t index) :
			    set(set),
			    index(index)
			{
			}

			const Item& get_item() const
			{
				return set->data[index];
			}

			const Key& operator*() const
			{
				return set->data[index].key;
			}

			const Key* operator->() const
			{
				return &set->data[index].key;
			}

			bool operator==(const const_iterator& other) const
			{
				return set == other.set && index == other.index;
			}

			bool operator!=(const const_iterator& other) const
			{
				return set != other.set || index != other.index;
			}

			const_iterator& operator++()
			{
				const size_t size = set->data.size();

				do
				{
					index++;
				} while (index < size && set->eq(set->data[index].key, set->empty_key));

				return *this;
			}

			const_iterator operator++(int)
			{
				const_iterator res = *this;
				++*this;
				return res;
			}

		private:
			const DenseHashTable* set;
			size_t index;
		};

	private:
		std::vector<Item> data;
		size_t count;
		Key empty_key;
		Hash hasher;
		Eq eq;

		void rehash()
		{
			size_t newsize = data.empty() ? 16 : data.size() * 2;
			DenseHashTable newtable(empty_key, newsize);

			for (size_t i = 0; i < data.size(); ++i)
				if (!eq(data[i].key, empty_key))
					*newtable.insert(data[i].key) = data[i];
			data.swap(newtable.data);
		}
	};

	template<typename Key, typename Value, typename Hash = std::hash<Key>, typename Eq = std::equal_to<Key> >
	class DenseHashMap
	{
		typedef DenseHashTable<Key, DenseHashMapItem<Key, Value>, Hash, Eq> Impl;
		Impl impl;

	public:
		typedef Impl::const_iterator const_iterator;

		DenseHashMap(const Key& empty_key, size_t buckets = 0) :
		    impl(empty_key, buckets)
		{
		}

		void clear()
		{
			impl.clear();
		}

		Value& operator[](const Key& key)
		{
			return impl.insert(key)->value;
		}

		const Value* find(const Key& key) const
		{
			const DenseHashMapItem<Key, Value>* result = impl.find(key);

			return result ? &result->value : NULL;
		}

		Value* find(const Key& key)
		{
			const DenseHashMapItem<Key, Value>* result = impl.find(key);

			return result ? const_cast<Value*>(&result->value) : NULL;
		}

		bool contains(const Key& key) const
		{
			return impl.find(key) != 0;
		}

		size_t size() const
		{
			return impl.size();
		}

		bool empty() const
		{
			return impl.size() == 0;
		}

		size_t bucket_count() const
		{
			return impl.bucket_count();
		}

		const_iterator begin() const
		{
			return impl.begin();
		}

		const_iterator end() const
		{
			return impl.end();
		}
	};
}
#pragma once

namespace Util {

template <typename T>
struct List
{
	u32 capacity; // Allocated capacity
	u32 length; // Length of list
	T *data;

	bool initalized; // Only in debug.

	List() : capacity(0), length(0), data(0), initalized(0) {};

	// Pointer math wrapper so it can be slotted in seamlessly
	T *operator+ (u32 i)
	{
		ASSERT(initalized, "Trying to use uninitalized list");
		return get_ptr(i);
	}

	// Normal indexing
	T operator[](u32 i) const
	{
		ASSERT(initalized, "Trying to use uninitalized list");
		return get(i);
	}

	// Reference indexing
	T &operator[](u32 i)
	{
		ASSERT(initalized, "Trying to use uninitalized list");
		return data[i];
	}

	void clear()
	{
		ASSERT(initalized, "Trying to use uninitalized list");
		length = 0;
	}

	void resize(u32 new_size)
	{
		ASSERT(initalized, "Trying to use uninitalized list");
		// Only expand it. No need to free memory.
		if (capacity < new_size)
		{
			data = resize_memory<T>(data, sizeof(T) * new_size);
			capacity = new_size;
		}
	}

	void append(T element)
	{
		ASSERT(initalized, "Trying to use uninitalized list");
		length++;
		if (length >= capacity)
			resize(capacity * 2);

		data[length - 1] = element;
	}

	T get(u32 i)
	{
		ASSERT(initalized, "Trying to use uninitalized list");
		return data[i];
	}

	T *get_ptr(u32 i)
	{
		ASSERT(initalized, "Trying to use uninitalized list");
		return data + i;
	}

	T set(u32 i, T element)
	{
		ASSERT(initalized, "Trying to use uninitalized list");
		return data[i] = element;
	}

	void insert(u32 i, T element)
	{
		ASSERT(initalized, "Trying to use uninitalized list");
		ASSERT(i <= length, "Invalid insert");
		append(element);

		for (u32 j = length - 1; i < j; j--)
			set(j, get(j - 1));
		set(i, element);
	}

	T remove(u32 i)
	{
		ASSERT(initalized, "Trying to use uninitalized list");
		T element = data[i];
		for (;i < length - 1; i++)
			data[i] = data[i + 1];

		length--;
		return element;
	}

};

template <typename T>
List<T> create_list(u32 capacity)
{
	List<T> list = {};
	list.capacity = capacity;
	list.data = Util::push_memory<T>(capacity);
	list.initalized = true;
	return list;
}

template <typename T>
void destroy_list(List<T> *list)
{
    Util::pop_memory(list->data);
	list = nullptr;
}

template <typename T>
List<T> concat(List<T> a, List<T> b)
{
	List<T> result = create_list<T>(a.length + b.length);
	result.length = result.capacity;

	for (u32 i = 0; i < a.length; i++)
		result[i] = a[i];

	for (u32 i = 0; i < b.length; i++)
		result[a.length + i] = b[i];

	return result;
}

};

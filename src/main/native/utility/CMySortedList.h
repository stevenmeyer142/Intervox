// CMySortedList.h
// Created by Steve on Fri, Mar 19, 1999 @ 2:43 PM.

#ifndef __CMySortedList__
#define __CMySortedList__

#include <vector>

enum CompareResult
{
	kItem1LessThanItem2_AC = -1,
	kItem1EqualItem2_AC = 0,
	kItem1GreaterThanItem2_AC = 1
};

enum
{
	kEmptyIndex_AC = 0

};

typedef std::vector<void *> MyVector;

class CMySortedList
{
	MyVector fVector;

	void DebugMessage(const char *message);

	ListIndex BinarySearch(const void *item, bool &found);

public:
	bool fDebug;

	CMySortedList();

	CMySortedList(ListIndex initialSize);

	void SetDebug(bool debug) { fDebug = debug; }

	virtual ~CMySortedList();

	virtual ListIndex GetSize();

	virtual void *At(ListIndex index);

	virtual CompareResult Compare(const void *item1, const void *item2);

	virtual void InsertLast(const void *item); // replace insert last with push

	virtual void InsertElementAt(void *item, ListIndex index, bool dontsort);

	virtual void Push(const void *item);

	virtual void *Pop();

	void RemoveAllItems();

	virtual ListIndex GetInsertIndexOf(const void *inItem, bool &found);

	virtual void *GetElementAt(ListIndex index);

	virtual void RemoveElementAt(ListIndex index);

	virtual void RemoveElement(const void *item);
};

#endif

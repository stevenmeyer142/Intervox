#include "NativeOpenGL.h"
#include "utility/CMySortedList.h"
#include <stdio.h>
#ifdef INTERVOX_JNI
#include "JNICommon.h"
#endif
#include <iterator>

#define kDebugList 0

// TODO: Change this code and its clear
CMySortedList::CMySortedList() : fVector(200), fDebug(false)
{
    fVector.clear();
    unsigned long size = fVector.size();
    if (size != 0)
    {
        DebugMessage("boo boo");
    }
}

CMySortedList::~CMySortedList()
{
}

void CMySortedList::DebugMessage(const char *message)
{
    if (fDebug)
    {
        ::printf("%s size%d\n", message, (int)fVector.size());
    }
}

CMySortedList::CMySortedList(ListIndex initialSize) : fVector(initialSize), fDebug(false)
{
    fVector.clear();
    unsigned long size = fVector.size();

    if (size != 0)
    {
        DebugMessage("boo boo");
    }
}

CompareResult CMySortedList::Compare(const void *item1, const void *item2)
{
    CompareResult result = kItem1EqualItem2_AC;
    if (item1 > item2)
    {
        result = kItem1GreaterThanItem2_AC;
    }
    else if (item1 < item2)
    {
        result = kItem1LessThanItem2_AC;
    }

    return result;
}

ListIndex CMySortedList::GetSize()
{
    return fVector.size();
}

void *CMySortedList::At(ListIndex index)
{
    return GetElementAt(index);
}

void CMySortedList::InsertLast(const void *item)
{
    fVector.push_back((void *)item);
}

void CMySortedList::InsertElementAt(void *item, ListIndex index, bool /* dontsort */)
{

    MyVector::iterator iter(fVector.begin());

    iter += index - 1;

    fVector.insert(iter, item);
}

void CMySortedList::Push(const void *item)
{
    InsertLast(item);
}

void *CMySortedList::Pop()
{
    void *result = NULL;

    if (!fVector.empty())
    {
        result = fVector.back();
        fVector.pop_back();
    }
    return result;
}

void CMySortedList::RemoveAllItems()
{
    fVector.clear();
}

ListIndex CMySortedList::GetInsertIndexOf(const void *inItem, bool &found)
{
    ListIndex result = 1;
    ListIndex count = this->GetSize();
    found = false;

    if (count > 0)
    {
        result = BinarySearch(inItem, found);

        result++; // index starts at 1 not 0
    }
    if (fDebug)
    {
        ::printf("CMySortedList::GetInsertIndexOf, Index %d, found %s \n", (int)result, found ? "t" : "F");
    }

    return result;
}

void *CMySortedList::GetElementAt(ListIndex index)
{
    void *result = NULL;

    if (fDebug && false)
    {
        ::printf("CMySortedList::GetElementAt, Index %d  \n", (int)index);
    }

    if (!fVector.empty() && index <= (ListIndex)fVector.size())
    {
        result = fVector[index - 1];
    }

    return result;
}

void CMySortedList::RemoveElementAt(ListIndex index)
{
    if (fDebug)
    {
        ::printf("CMySortedList::RemoveElementA, Index %d  \n", (int)index);
    }

    if (!fVector.empty() && index <= (ListIndex)fVector.size())
    {
        MyVector::iterator iter(fVector.begin());

        iter += index - 1;

        fVector.erase(iter);
    }
}

void CMySortedList::RemoveElement(const void *item)
{
    bool found;
    ListIndex index = GetInsertIndexOf(item, found);

    if (found)
    {
        RemoveElementAt(index);
    }
}

ListIndex CMySortedList::BinarySearch(const void *item, bool &found)
{
    ListIndex lower = 0;
    ListIndex upper = fVector.size() - 1;
    found = false;
    ListIndex middle = 0;
    while (lower <= upper)
    {
        middle = (lower + upper) / 2;
        CompareResult compare = Compare(item, fVector[middle]);

        if (compare == kItem1GreaterThanItem2_AC)
            lower = middle + 1;
        else if (compare == kItem1LessThanItem2_AC)
            upper = middle - 1;
        else // kItem1EqualItem2_AC
        {
            found = true;
            break;
        }

        if (fDebug)
        {
            ::printf("BinarySearch, middle %d, lower %d, upper %d, compare %d\n", (int)middle, (int)lower, (int)upper, (int)compare);
        }
    }

    if (!found)
    {
        middle = lower;
    }
    if (fDebug)
    {
        ::printf("BinarySearch, found %d, index %d\n", found, (int)middle);
    }
    return middle;
}

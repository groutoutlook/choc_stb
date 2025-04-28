#ifndef CHOC_DIRTY_LIST_HEADER_INCLUDED
#define CHOC_DIRTY_LIST_HEADER_INCLUDED

#include <atomic>
#include <vector>
#	include <mutex>

// If the project doesn't define a custom implementation for CHOC_ASSERT, the default
// behaviour is just to call the normal system assert()
#ifndef CHOC_ASSERT
#	include <cassert>
#	define CHOC_ASSERT(x) assert(x);
#endif

// It's never a smart idea to include any C headers before your C++ ones, as they
// often pollute your namespace with all kinds of dangerous macros like these ones.
// This file is included by many of the choc headers, so is a convenient place to
// undef these.
#undef max
#undef min

namespace choc::threading
{

//==============================================================================
/**
    A minimal no-frills spin-lock.

    To use an RAII pattern for locking a SpinLock, it's compatible with the normal
    scoped guard locking classes in the standard library.
*/
struct SpinLock
{
	SpinLock()  = default;
	~SpinLock() = default;

	void lock();
	bool try_lock();
	void unlock();

  private:
	std::atomic_flag flag = ATOMIC_FLAG_INIT;
};

//==============================================================================
//        _        _           _  _
//     __| |  ___ | |_   __ _ (_)| | ___
//    / _` | / _ \| __| / _` || || |/ __|
//   | (_| ||  __/| |_ | (_| || || |\__ \ _  _  _
//    \__,_| \___| \__| \__,_||_||_||___/(_)(_)(_)
//
//   Code beyond this point is implementation detail...
//
//==============================================================================

inline void SpinLock::lock()
{
	while (flag.test_and_set(std::memory_order_acquire))
	{
	}
}
inline bool SpinLock::try_lock()
{
	return !flag.test_and_set(std::memory_order_acquire);
}
inline void SpinLock::unlock()
{
	flag.clear();
}

}        // namespace choc::threading

namespace choc::fifo
{

//==============================================================================
/**
    Manages the read and write positions for a FIFO (but not the storage of
    objects in a FIFO).
*/
template <typename IndexType, typename AtomicType>
struct FIFOReadWritePosition
{
	FIFOReadWritePosition();
	~FIFOReadWritePosition() = default;

	//==============================================================================
	static constexpr IndexType invalidIndex = std::numeric_limits<IndexType>::max();

	//==============================================================================
	/// Resets the positions and initialises the number of items in the FIFO.
	void reset(size_t numItems);

	/// Resets the FIFO positions, keeping the current size.
	void reset();

	/// Returns the total number of items that the FIFO has been set up to hold.
	IndexType getTotalCapacity() const
	{
		return capacity;
	}
	/// Returns the number of items in the FIFO.
	IndexType getUsedSlots() const
	{
		return getUsed(readPos, writePos);
	}
	/// Returns the number of free slots in the FIFO.
	IndexType getFreeSlots() const
	{
		return getFree(readPos, writePos);
	}

	//==============================================================================
	struct WriteSlot
	{
		/// Returns true if a free slot was successfully obtained.
		operator bool() const
		{
			return index != invalidIndex;
		}

		/// The index of the slot that should be written to.
		IndexType index;

	  private:
		friend struct FIFOReadWritePosition;
		IndexType newEnd;
	};

	/// Attempts to get a slot into which the next item can be pushed.
	/// The WriteSlot object that is returned must be checked for validity by using its
	/// cast to bool operator - if the FIFO is full, it will be invalid. If it's valid
	/// then the caller must read what it needs from the slot at the index provided, and
	/// then immediately afterwards call unlock() to release the slot.
	WriteSlot lockSlotForWriting();

	/// This must be called immediately after writing an item into the slot provided by
	/// lockSlotForWriting().
	void unlock(WriteSlot);

	//==============================================================================
	struct ReadSlot
	{
		/// Returns true if a readable slot was successfully obtained.
		operator bool() const
		{
			return index != invalidIndex;
		}

		/// The index of the slot that should be read.
		IndexType index;

	  private:
		friend struct FIFOReadWritePosition;
		IndexType newStart;
	};

	/// Attempts to get a slot from which the first item can be read.
	/// The ReadSlot object that is returned must be checked for validity by using its
	/// cast to bool operator - if the FIFO is empty, it will be invalid. If it's valid
	/// then the caller must read what it needs from the slot at the index provided, and
	/// then immediately afterwards call unlock() to release the slot.
	ReadSlot lockSlotForReading();

	/// This must be called immediately after reading an item from the slot provided by
	/// lockSlotForReading().
	void unlock(ReadSlot);

  private:
	//==============================================================================
	uint32_t   capacity = 0;
	AtomicType readPos, writePos;

	uint32_t getUsed(uint32_t s, uint32_t e) const
	{
		return e >= s ? (e - s) : (capacity + 1u - (s - e));
	}
	uint32_t getFree(uint32_t s, uint32_t e) const
	{
		return e >= s ? (capacity + 1u - (e - s)) : (s - e);
	}
	uint32_t increment(uint32_t i) const
	{
		return i != capacity ? i + 1u : 0;
	}

	FIFOReadWritePosition(const FIFOReadWritePosition &) = delete;
};

//==============================================================================
//        _        _           _  _
//     __| |  ___ | |_   __ _ (_)| | ___
//    / _` | / _ \| __| / _` || || |/ __|
//   | (_| ||  __/| |_ | (_| || || |\__ \ _  _  _
//    \__,_| \___| \__| \__,_||_||_||___/(_)(_)(_)
//
//   Code beyond this point is implementation detail...
//
//==============================================================================

template <typename IndexType, typename AtomicType>
FIFOReadWritePosition<IndexType, AtomicType>::FIFOReadWritePosition()
{
	reset(1);
}

template <typename IndexType, typename AtomicType>
void FIFOReadWritePosition<IndexType, AtomicType>::reset(size_t size)
{
	capacity = static_cast<uint32_t>(size);
	reset();
}

template <typename IndexType, typename AtomicType>
void FIFOReadWritePosition<IndexType, AtomicType>::reset()
{
	readPos  = 0;
	writePos = 0;
}

template <typename IndexType, typename AtomicType>
typename FIFOReadWritePosition<IndexType, AtomicType>::WriteSlot FIFOReadWritePosition<IndexType, AtomicType>::lockSlotForWriting()
{
	WriteSlot slot;
	slot.index  = writePos.load();
	slot.newEnd = increment(slot.index);

	if (slot.newEnd == readPos)
		slot.index = invalidIndex;

	return slot;
}

template <typename IndexType, typename AtomicType>
void FIFOReadWritePosition<IndexType, AtomicType>::unlock(WriteSlot slot)
{
	writePos = slot.newEnd;
}

template <typename IndexType, typename AtomicType>
typename FIFOReadWritePosition<IndexType, AtomicType>::ReadSlot FIFOReadWritePosition<IndexType, AtomicType>::lockSlotForReading()
{
	ReadSlot slot;
	slot.index = readPos.load();

	if (slot.index == writePos)
	{
		slot.index    = invalidIndex;
		slot.newStart = slot.index;
	}
	else
	{
		slot.newStart = increment(slot.index);
	}

	return slot;
}

template <typename IndexType, typename AtomicType>
void FIFOReadWritePosition<IndexType, AtomicType>::unlock(ReadSlot slot)
{
	readPos = slot.newStart;
}

}        // namespace choc::fifo

namespace choc::fifo
{

//==============================================================================
/**
    A simple atomic single-reader, single-writer FIFO.
*/
template <typename Item>
struct SingleReaderSingleWriterFIFO
{
	SingleReaderSingleWriterFIFO();
	~SingleReaderSingleWriterFIFO() = default;

	/** Clears the FIFO and allocates a size for it. */
	void reset(size_t numItems);

	/** Clears the FIFO and allocates a size for it, filling the slots with
	    copies of the given object.
	    Note that this is not thread-safe with respect to the other methods - it must
	    only be called when nothing else is modifying the FIFO.
	*/
	void reset(size_t numItems, const Item &itemInitialiser);

	/** Resets the FIFO, keeping the current size. */
	void reset()
	{
		position.reset();
	}

	/** Returns the number of items in the FIFO. */
	uint32_t getUsedSlots() const
	{
		return position.getUsedSlots();
	}
	/** Returns the number of free slots in the FIFO. */
	uint32_t getFreeSlots() const
	{
		return position.getFreeSlots();
	}

	/** Attempts to push an into into the FIFO, returning false if no space was available. */
	bool push(const Item &);

	/** Attempts to push an into into the FIFO, returning false if no space was available. */
	bool push(Item &&);

	/** If any items are available, this copies the first into the given target, and returns true. */
	bool pop(Item &result);

  private:
	FIFOReadWritePosition<uint32_t, std::atomic<uint32_t>> position;
	std::vector<Item>                                      items;

	SingleReaderSingleWriterFIFO(const SingleReaderSingleWriterFIFO &) = delete;
};

//==============================================================================
//        _        _           _  _
//     __| |  ___ | |_   __ _ (_)| | ___
//    / _` | / _ \| __| / _` || || |/ __|
//   | (_| ||  __/| |_ | (_| || || |\__ \ _  _  _
//    \__,_| \___| \__| \__,_||_||_||___/(_)(_)(_)
//
//   Code beyond this point is implementation detail...
//
//==============================================================================

template <typename Item>
SingleReaderSingleWriterFIFO<Item>::SingleReaderSingleWriterFIFO()
{
	reset(1);
}

template <typename Item>
void SingleReaderSingleWriterFIFO<Item>::reset(size_t size)
{
	position.reset(size);
	items.resize(size + 1u);
}

template <typename Item>
void SingleReaderSingleWriterFIFO<Item>::reset(size_t size, const Item &itemToCopy)
{
	position.reset(size);
	items.resize(size + 1u, itemToCopy);
}

template <typename Item>
bool SingleReaderSingleWriterFIFO<Item>::push(const Item &item)
{
	if (auto slot = position.lockSlotForWriting())
	{
		items[slot.index] = item;
		position.unlock(slot);
		return true;
	}

	return false;
}

template <typename Item>
bool SingleReaderSingleWriterFIFO<Item>::push(Item &&item)
{
	if (auto slot = position.lockSlotForWriting())
	{
		items[slot.index] = std::move(item);
		position.unlock(slot);
		return true;
	}

	return false;
}

template <typename Item>
bool SingleReaderSingleWriterFIFO<Item>::pop(Item &result)
{
	if (auto slot = position.lockSlotForReading())
	{
		result = std::move(items[slot.index]);
		position.unlock(slot);
		return true;
	}

	return false;
}

}        // namespace choc::fifo

namespace choc::fifo
{

//==============================================================================
/**
    A simple atomic single-reader, multiple-writer FIFO.

    Note that the idea with this class is for it to be realtime-safe and lock-free
    for the reader, but the writers may very briefly block each other if more than
    one thread attempts to write at the same time.
*/
template <typename Item>
struct SingleReaderMultipleWriterFIFO
{
	SingleReaderMultipleWriterFIFO()  = default;
	~SingleReaderMultipleWriterFIFO() = default;

	/** Clears the FIFO and allocates a size for it.
	    Note that this is not thread-safe with respect to the other methods - it must
	    only be called when nothing else is modifying the FIFO.
	*/
	void reset(size_t numItems)
	{
		fifo.reset(numItems);
	}

	/** Clears the FIFO and allocates a size for it, filling the slots with
	    copies of the given object.
	*/
	void reset(size_t numItems, const Item &itemInitialiser)
	{
		fifo.reset(numItems, itemInitialiser);
	}

	/** Resets the FIFO, keeping the current size. */
	void reset()
	{
		fifo.reset();
	}

	/** Returns the number of items in the FIFO. */
	uint32_t getUsedSlots() const
	{
		return fifo.getUsedSlots();
	}
	/** Returns the number of free slots in the FIFO. */
	uint32_t getFreeSlots() const
	{
		return fifo.getFreeSlots();
	}

	/** Attempts to push an into into the FIFO, returning false if no space was available. */
	bool push(const Item &);

	/** Attempts to push an into into the FIFO, returning false if no space was available. */
	bool push(Item &&);

	/** If any items are available, this copies the first into the given target, and returns true. */
	bool pop(Item &result)
	{
		return fifo.pop(result);
	}

  private:
	choc::fifo::SingleReaderSingleWriterFIFO<Item> fifo;
	choc::threading::SpinLock                      writeLock;

	SingleReaderMultipleWriterFIFO(const SingleReaderMultipleWriterFIFO &) = delete;
};

//==============================================================================
//        _        _           _  _
//     __| |  ___ | |_   __ _ (_)| | ___
//    / _` | / _ \| __| / _` || || |/ __|
//   | (_| ||  __/| |_ | (_| || || |\__ \ _  _  _
//    \__,_| \___| \__| \__,_||_||_||___/(_)(_)(_)
//
//   Code beyond this point is implementation detail...
//
//==============================================================================

template <typename Item>
bool SingleReaderMultipleWriterFIFO<Item>::push(const Item &item)
{
	const std::scoped_lock lock(writeLock);
	return fifo.push(item);
}

template <typename Item>
bool SingleReaderMultipleWriterFIFO<Item>::push(Item &&item)
{
	const std::scoped_lock lock(writeLock);
	return fifo.push(std::move(item));
}

}        // namespace choc::fifo

namespace choc::fifo
{

//==============================================================================
/**
    A lock-free list of objects where multiple threads may mark an object as dirty,
    while a single thread polls the list and service the dirty ones.

    The main use-case for this class is where a set of objects need to be asynchronously
    serviced by a single thread or timer after they get flagged by a realtime thread.

    The class is designed so that the markAsDirty() and popNextDirtyObject() functions
    will execute in very short, constant time even when the total number of objects is
    very large, and no heap allocations or other system calls are involved. And if only
    one thread ever calls markAsDirty(), it's safe for realtime use.
    To make this possible, the compromises are that it needs to be initialised with a
    complete list of the objects needed, so that it can assign handles to them, and its
    memory requirements include allocating a small amount of storage per object.
*/
template <typename ObjectType>
struct DirtyList
{
	DirtyList()  = default;
	~DirtyList() = default;

	DirtyList(DirtyList &&)      = delete;
	DirtyList(const DirtyList &) = delete;

	using Handle = uint32_t;

	/// Prepares the list by giving it the complete set of objects that it will manage.
	/// The return value is the set of handles assigned to each of the objects. The handles
	/// are later needed by the caller in order to call markAsDirty().
	///
	/// Note that this method is not thread-safe, and must be performed before any other
	/// operations begin. It can be called multiple times to re-initialise the same list
	/// for other objects, as long as thread-safety is observed.
	template <typename Array>
	std::vector<Handle> initialise(const Array &objects);

	/// Clears the queue of pending items and resets the 'dirty' state of all objects.
	void resetAll();

	/// Marks an object as dirty.
	///
	/// This may be called from any thread, and is lock-free.
	/// If the object is already marked as dirty, this function does nothing. If not, then
	/// the object is marked as dirty and added to the queue of objects which will later
	/// be returned by calls to popNextDirtyObject().
	void markAsDirty(Handle objectHandle);

	/// Returns a pointer to the next dirty object (and in doing so marks that object
	/// as now being 'clean').
	/// If no objects are dirty, this returns nullptr.
	/// This method is lock-free, but is designed to be called by only a single reader
	/// thread.
	ObjectType *popNextDirtyObject();

	/// Returns true if any objects are currently queued for attention.
	bool areAnyObjectsDirty() const;

  private:
	//==============================================================================
	std::unique_ptr<std::atomic_flag[]>                flags;        // avoiding a vector here as atomics aren't copyable
	std::vector<ObjectType *>                          allObjects;
	choc::fifo::SingleReaderMultipleWriterFIFO<Handle> fifo;
};

//==============================================================================
//        _        _           _  _
//     __| |  ___ | |_   __ _ (_)| | ___
//    / _` | / _ \| __| / _` || || |/ __|
//   | (_| ||  __/| |_ | (_| || || |\__ \ _  _  _
//    \__,_| \___| \__| \__,_||_||_||___/(_)(_)(_)
//
//   Code beyond this point is implementation detail...
//
//==============================================================================

template <typename ObjectType>
template <typename Array>
std::vector<typename DirtyList<ObjectType>::Handle> DirtyList<ObjectType>::initialise(const Array &objects)
{
	std::vector<Handle> handles;
	auto                numObjects = static_cast<size_t>(objects.size());
	handles.reserve(numObjects);
	flags.reset(new std::atomic_flag[numObjects]);
	allObjects.resize(numObjects);
	fifo.reset(numObjects);
	size_t i = 0;

	for (auto &o : objects)
	{
		//CHOC_ASSERT(o != nullptr);
		flags[i].clear();
		allObjects[i] = o;
		handles.push_back(static_cast<Handle>(i));
		++i;
	}

	return handles;
}

template <typename ObjectType>
void DirtyList<ObjectType>::resetAll()
{
	for (auto &o : allObjects)
		o->isDirty = false;
		//o.isDirty = false;

	fifo.reset();
}

template <typename ObjectType>
void DirtyList<ObjectType>::markAsDirty(Handle objectHandle)
{
	CHOC_ASSERT(objectHandle < allObjects.size());

	if (!flags[objectHandle].test_and_set())
		fifo.push(objectHandle);
}

template <typename ObjectType>
ObjectType *DirtyList<ObjectType>::popNextDirtyObject()
{
	Handle item;

	if (!fifo.pop(item))
		return nullptr;

	flags[item].clear();
	return allObjects[item];
}

template <typename ObjectType>
bool DirtyList<ObjectType>::areAnyObjectsDirty() const
{
	return fifo.getUsedSlots() != 0;
}

}        // namespace choc::fifo

#endif
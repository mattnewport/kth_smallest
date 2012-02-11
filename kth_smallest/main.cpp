#include <algorithm>
#include <numeric>
#include <vector>
#include <array>
#include <iostream>
#include <cstdint>
#include <cassert>

#include <Windows.h>

using namespace std;

static const size_t NumElements = 128 * 1024;
static const size_t K = 256;

template<typename T, size_t K_>
class KSmallest {
public:
	KSmallest() : numVals(0) {}

	void Insert(const T& val) {
		const auto insertIt = lower_bound(begin(vals), begin(vals) + numVals, val);
		if (insertIt != end(vals)) {
			numVals = min(numVals + 1, K_);
			for (auto dest = begin(vals) + numVals - 1; dest > insertIt; --dest) {
				*dest = *(dest - 1);
			}
			*insertIt = val;
		}
	}

	T KthSmallest() { return vals[K_ - 1]; }

private:
	size_t numVals;
	T vals[K];
};

uint32_t FindKthSmallest(const uint32_t* values) {
	KSmallest<uint32_t, K> finder;
	for_each(values, values + NumElements, [&](const uint32_t x) { finder.Insert(x); });
	
	return finder.KthSmallest();
}

uint32_t FindKthSmallestHeap(const uint32_t* values) {
	static_assert(K <= NumElements, "K must be less than or equal to NumElements.");
	array<uint32_t, K> maxHeap;
	copy(values, values + K, maxHeap.begin());
	make_heap(begin(maxHeap), end(maxHeap));
	for_each(values + K, values + NumElements, [&](const uint32_t x) {
		if (x < maxHeap.front()) {
			pop_heap(begin(maxHeap), end(maxHeap));
			maxHeap.back() = x;
			push_heap(begin(maxHeap), end(maxHeap));
		}
	});
	return maxHeap.front();
}

template<typename T, size_t K_>
class MaxHeap {
public:
	explicit MaxHeap(const uint32_t* values) : numElems(K) {
		copy(values, values + K, begin(heap));
		make_heap(begin(heap), end(heap));
	}

	const T& Top() { return *begin(heap); }

	void Add(const T& val) {
		if (val >= Top()) {
			return;
		}
		Remove();
		Insert(val);
	}

private:
	void Insert(const T& val) {
		size_t idx = numElems++;
		heap[idx] = val;
		while (idx) {
			const auto parentIdx = Parent(idx);
			if (heap[idx] <= heap[parentIdx]) {
				break;
			}
			swap(heap[idx], heap[parentIdx]);
			idx = parentIdx;
		}
	}

	void Remove() {
		*begin(heap) = heap[--numElems];
		size_t parentIdx = 0;
		size_t childIdx = 2 * parentIdx + 2;
		while (childIdx < numElems) {
			if (heap[childIdx] < heap[childIdx - 1]) {
				--childIdx;
			}
			if (heap[parentIdx] >= heap[childIdx]) {
				break;
			}
			swap(heap[parentIdx], heap[childIdx]);
			parentIdx = childIdx;
			childIdx = 2 * parentIdx + 2;
		}

		if (childIdx == numElems) { // only child at bottom
			if (heap[parentIdx] < heap[childIdx - 1]) {
				swap(heap[parentIdx], heap[childIdx - 1]);
			}
		}
	}

	size_t Parent(size_t idx) { return (idx - 1) / 2; }

	size_t numElems;
	array<T, K_> heap;
};

uint32_t FindKthSmallestCustomHeap(const uint32_t* values) {
	MaxHeap<uint32_t, K> maxHeap(values);
	for_each(values + K, values + NumElements, [&](const uint32_t x) {
		maxHeap.Add(x);
	});
	return maxHeap.Top();
}

class Timer {
public:
	Timer() { QueryPerformanceCounter(&startTime); }
	double GetElapsedTime() {
		LARGE_INTEGER endTime;
		QueryPerformanceCounter(&endTime);
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		return double(endTime.QuadPart - startTime.QuadPart) / double(frequency.QuadPart);
	}
private:
	LARGE_INTEGER startTime;
};

template<typename Func>
double TestFindKthSmallest(Func findFunc, const char* funcName) {
	static const int NumIterations = 100;
	double totalTime = 0.0f;
	for (int i = 0; i != NumIterations; ++i) {
		vector<uint32_t> v(NumElements);
		iota(begin(v), end(v), 0);
		random_shuffle(begin(v), end(v));

		Timer timer;
		auto kthSmallest = findFunc(&v[0]);
		auto time = timer.GetElapsedTime();
		totalTime += time;
		assert(kthSmallest == K - 1);

		cout << funcName << "(): kth smallest value: " << kthSmallest << ", took " << time << "s." << endl;
	}
	return totalTime / NumIterations;
}

#define TEST(f) { averageTimes.push_back(make_pair(#f, TestFindKthSmallest(f, #f))); }

int main() {
	typedef pair<const char*, double> TestResult;
	vector<TestResult> averageTimes;
	TEST(FindKthSmallest);
	TEST(FindKthSmallestHeap);
	TEST(FindKthSmallestCustomHeap);
	for_each(begin(averageTimes), end(averageTimes), [&](const TestResult& r) {
		cout << r.first << "(): average time was " << r.second << "s." << endl;
	});

	return 0;
}
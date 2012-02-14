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
	explicit MaxHeap(const T* values) : numElems(K) {
		copy(values, values + K, begin(heap) + 1);
		make_heap(begin(heap) + 1, end(heap));
	}

	const T& Top() { return heap[1]; }

	void ReplaceTop(const T& val) {
		auto hole = PopHeap();
		PushHeap(val, hole);
	}

private:
	size_t PopHeap() {
		size_t hole = 1;
		auto child = 2 * hole;
		while (child < K_) {
			const auto rChild = child + 1;
			const auto lChildVal = heap[child];
			const auto rChildVal = heap[rChild];
			heap[hole] = max(lChildVal, rChildVal);
			hole = lChildVal > rChildVal ? child : rChild;
			child = 2 * hole;
		}

		if (child == K_) { // only child at bottom
			heap[hole] = heap[child];
			hole = child;
		}

		return hole;
	}

	void PushHeap(const T& val, size_t hole) {
		while (hole > 1) {
			const auto parent = hole / 2;
			if (val < heap[parent]) {
				break;
			}
			heap[hole] = heap[parent];
			hole = parent;
		}
		heap[hole] = val;
	}

	size_t numElems;
	array<T, K_ + 1> heap; // K_ + 1 because heap is 1 rather than 0 indexed
};

uint32_t FindKthSmallestCustomHeap(const uint32_t* values) {
	MaxHeap<uint32_t, K> maxHeap(values);
	for_each(values + K, values + NumElements, [&](const uint32_t x) {
		if (x < maxHeap.Top()) {
			maxHeap.ReplaceTop(x);
		}
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

		cout << funcName << "(): kth smallest value: " << kthSmallest << ", took " << (time * 1000.0f) << "ms." << endl;
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
		cout << r.first << "(): average time was " << (r.second * 1000.0f) << "ms." << endl;
	});

	return 0;
}
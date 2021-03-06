#include "common.h"





template<template<typename> class TQueue>
uint64 benchmark_vector(size_t numValues, size_t capacity)
{
  using intlist = std::vector<int>;

  TQueue<intlist> queue(capacity);
  std::vector<intlist> source;
  std::vector<intlist> dest;
  dest.reserve(numValues);
  source.reserve(numValues);

  for (size_t i = 0; i < numValues; ++i)
  {
    source.push_back(intlist(64, 0));
  }

  auto produce = [&]() {
    const size_t local_num = numValues;
    
    for (size_t i = 0; i < local_num; ++i)
    {
      while (!queue.write(std::move(source[i])))
      {
        std::this_thread::yield();
      }
    }
  };

  auto consume = [&]() {
    const size_t local_num = numValues;

    intlist tmp;
    for (size_t i = 0; i < local_num; ++i)
    { 
      while (!queue.read(tmp))
      {
        std::this_thread::yield();
      }
      dest.push_back(std::move(tmp));
    }
  };

  auto start = teetime::platform::microSeconds();
  std::thread consumer(consume);
  std::thread producer(produce);
  producer.join();
  consumer.join();
  auto end = teetime::platform::microSeconds();

  const intlist s(64, 0);
  for (size_t i = 0; i < dest.size(); ++i)
  {
    if (dest[i] != s) {
      std::cout << "ERROR: " << i << ":" << dest[i].size() << std::endl;
      break;
    }
  }

  return (end - start);
}



int main(int argc, char** argv)
{
  setLogCallback(::teetime::simpleLogging);
  setLogLevel(getLogLevelFromArgs(argc, argv));
  size_t iterations = 3;
  size_t capacity = 1024;

  int numValues = 10000000;

  Params params = getQueueType(argc, argv, "queue_vector");
  switch (params.type) {
  case QueueType::Folly:
    run(iterations, numValues, capacity, "Folly", benchmark_vector<folly::ProducerConsumerQueue>, params.filename);
    break;
  case QueueType::FollyOpt:
    run(iterations, numValues, capacity, "FollyOpt", benchmark_vector<folly::AlignedProducerConsumerQueue>, params.filename);
    break;
  case QueueType::Boost:
#ifdef TEETIME_HAS_BOOST
    run(iterations, numValues, capacity, "boost::spsc_queue", benchmark_vector<BoostSpscQueue>, params.filename);
    break;
#else
    std::cout << "boost not available" << std::endl;
    return EXIT_FAILURE;
#endif    
  case QueueType::TeeTimePointer:
    std::cout << "values not supported" << std::endl;
    return EXIT_FAILURE;
  case QueueType::TeeTimeValue:
    run(iterations, numValues, capacity, "SpscValueQueue", benchmark_vector<SpscValueQueue>, params.filename);
    break;
  case QueueType::FastFlow:
    std::cout << "values not supported" << std::endl;
    return EXIT_FAILURE;
  case QueueType::FastFlowNoVolatile:
    std::cout << "values not supported" << std::endl;
    return EXIT_FAILURE;
  default:
    std::cout << "unknown or unsupported queue type" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

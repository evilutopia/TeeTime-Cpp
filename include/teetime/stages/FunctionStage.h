/**
* Copyright (C) 2016 Johannes Ohlemacher (https://github.com/eXistence/TeeTime-Cpp)
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*         http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#pragma once
#include <teetime/stages/AbstractConsumerStage.h>
#include <teetime/stages/AbstractFilterStage.h>
#include <functional>

namespace teetime
{
  namespace internal
  {
    template<typename T>
    struct function_traits;

    template<typename R, typename ...Args>
    struct function_traits<std::function<R(Args...)>>
    {
      static const size_t nargs = sizeof...(Args);

      typedef R result_type;

      template <size_t i>
      struct arg
      {
        typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
      };
    };

  }

  template<typename TIn, typename TOut, TOut(*TFunc)(TIn)>
  class FunctionStage final : public AbstractConsumerStage<TIn>
  {
  public:
    explicit FunctionStage(const char* debugName = "Function")
      : AbstractConsumerStage<TIn>(debugName)
    {
      m_outputPort = AbstractStage::addNewOutputPort<TOut>();
    }

    OutputPort<TOut>& getOutputPort()
    {
      assert(m_outputPort);
      return *m_outputPort;
    }

  private:
    virtual void execute(TIn&& in) override
    {
      assert(m_outputPort);
      m_outputPort->send(TFunc(std::move(in)));
    }

    OutputPort<TOut>* m_outputPort;
  };


  template<typename TIn, typename TOut>
  class FunctionPtrStage final : public AbstractConsumerStage<TIn>
  {
  public:
    typedef TOut(*Function)(TIn in);

    explicit FunctionPtrStage(Function f, const char* debugName = "FunctionPtr")
      : AbstractConsumerStage<TIn>(debugName)
      , m_f(f)
    {
      assert(m_f);
      m_outputPort = AbstractStage::addNewOutputPort<TOut>();
    }

    OutputPort<TOut>& getOutputPort()
    {
      assert(m_outputPort);
      return *m_outputPort;
    }

  private:
    virtual void execute(TIn&& in) override
    {
      assert(m_f);
      m_outputPort->send(m_f(std::move(in)));
    }

    const Function m_f;
    OutputPort<TOut>* m_outputPort;
  };


  template<typename TIn, typename TOut>
  class FunctionObjectStage final : public AbstractConsumerStage<TIn>
  {
  public:
    template<typename TFunc>
    explicit FunctionObjectStage(TFunc f, const char* debugName = "Function")
      : AbstractConsumerStage<TIn>(debugName)
      , m_function(f)
    {
      m_outputPort = AbstractStage::addNewOutputPort<TOut>();
    }

    OutputPort<TOut>& getOutputPort()
    {
      assert(m_outputPort);
      return *m_outputPort;
    }

  private:
    virtual void execute(TIn&& in) override
    {
      assert(m_outputPort);
      m_outputPort->send(m_function(std::move(in)));
    }

    std::function<TOut(TIn)> m_function;
    OutputPort<TOut>* m_outputPort;
  };




  template<typename TIn, typename TOut = TIn>
  class NewFunctionStage final : public AbstractFilterStage<TIn, TOut>
  {
  public:
    template<typename Callable>
    NewFunctionStage(Callable&& c)
      : m_function(c)
    {}

  private:
    virtual void execute(TIn&& value) override
    {
      AbstractFilterStage<TIn, TOut>::getOutputPort().send(m_function(std::move(value)));
    }

    std::function<TOut(TIn)> m_function;
  };
}

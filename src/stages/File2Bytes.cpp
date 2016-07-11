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

#include <teetime/stages/File2Bytes.h>
#include <teetime/ports/OutputPort.h>

#include <fstream>

using namespace teetime;

File2Bytes::File2Bytes(const char* debugName)
  : AbstractConsumerStage<File>(debugName)
  , m_outputPort(nullptr)
{
  m_outputPort = AbstractStage::addNewOutputPort<ByteArray>();
}

OutputPort<ByteArray>& File2Bytes::getOutputPort()
{
  assert(m_outputPort);
  return *m_outputPort;
}

void File2Bytes::execute(const File& value)
{
  std::ifstream file(value.path, std::ios::binary | std::ios::ate);

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  ByteArray buffer(size);
  if (file.read((char*)buffer.data(), size))
  {
    getOutputPort().send(buffer);
  }
  else
  {
    TEETIME_DEBUG() << "failed to read file: " << value.path;
  } 
}
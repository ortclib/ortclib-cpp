#pragma once

#include <zsLib/eventing/noop.h>
#ifdef _WIN32

#include "ortc.events_win.h"

#else

#include <zsLib/eventing/Log.h>
#include <stdint.h>

namespace zsLib {
  namespace eventing {
    
    template <typename TWriteType>
    void eventWriteBuffer(BYTE * &p, TWriteType value)
    {
      memcpy(&p, &value, sizeof(value));
      p += sizeof(value);
    }
    inline void eventWriteBuffer(BYTE ** &p, const BYTE *buffer, size_t * &bufferSizes, size_t size)
    {
      (*p) = const_cast<BYTE *>(buffer);
      (*bufferSizes) = size;
      ++p;
      ++bufferSizes;
    }
    inline void eventWriteBuffer(BYTE ** &p, const char *str, size_t * &bufferSizes)
    {
      (*p) = const_cast<BYTE *>(reinterpret_cast<const BYTE *>(str));
      (*bufferSizes) = (NULL == str ? 0 : strlen(str)) * sizeof(char);
      ++p;
      ++bufferSizes;
    }
    inline void eventWriteBuffer(const BYTE ** &p, const wchar_t *str, size_t * &bufferSizes)
    {
      (*p) = reinterpret_cast<const BYTE *>(str);
      (*bufferSizes) = (NULL == str ? 0 : wcslen(str)) * sizeof(wchar_t);
      ++p;
      ++bufferSizes;
    }
  }
}
#endif // _WIN32

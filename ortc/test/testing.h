/*
 
 Copyright (c) 2013, SMB Phone Inc.
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies,
 either expressed or implied, of the FreeBSD Project.
 
 */

#ifndef TESTING_AUTO_TEST_SUITE

#include <zsLib/types.h>
#include <sstream>
#include <iostream>

#ifdef __QNX__
#include <QDebug>
#endif //__QNX__

#ifdef _WIN32
// attribution to http://forum.valhallalegends.com/index.php?topic=5649.0

#include <vector>

template < typename T, class CharTraits = std::char_traits< T > >
class basic_debugstreambuf : private std::basic_streambuf< T, CharTraits >
{
public:
  typedef typename CharTraits::off_type off_type;
  typedef typename CharTraits::char_type char_type;
  typedef typename CharTraits::int_type int_type;
  typedef std::vector< typename T > BufferType;

public:
  basic_debugstreambuf() {
    _Init(NULL, NULL, NULL, &pBegin, &pCurrent, &pLength);
    m_outputBuffer.reserve(32);
  }

protected:
  virtual int_type overflow(int_type c = CharTraits::eof())
  {
    zsLib::AutoRecursiveLock lock(mLock);

    if (c == CharTraits::eof())
      return CharTraits::not_eof(c);

    m_outputBuffer.push_back(c);
    if (c == TEXT('\n'))
      sync();

    return c;
  }

  int sync()
  {
    zsLib::AutoRecursiveLock lock(mLock);

    m_outputBuffer.push_back(TEXT('\0'));
#ifdef _DEBUG
    OutputDebugStringA(reinterpret_cast< const T* >(&m_outputBuffer[0]));
#endif //_DEBUG
    std::cout << (reinterpret_cast< const T* >(&m_outputBuffer[0]));
    m_outputBuffer.clear();
    m_outputBuffer.reserve(32);
    return 0;
  }

protected:
  zsLib::RecursiveLock mLock;

  // put begin, put current and put length
  char_type* pBegin;
  char_type *pCurrent;
  int_type pLength;
  BufferType m_outputBuffer;
};

template < typename T, class CharTraits = std::char_traits< T > >
class basic_debugostream : public std::basic_ostream< T, CharTraits >
{
public:
  typedef basic_debugstreambuf< T, CharTraits > DebugStreamBuf;
  typedef std::basic_streambuf< T, CharTraits > StreamBuf;
  typedef std::basic_ostream< T, CharTraits > OStream;

public:
  basic_debugostream() : OStream((StreamBuf*)&m_streamBuffer)
  {
    clear();
  }

protected:
  DebugStreamBuf m_streamBuffer;
};

typedef basic_debugostream< char > debugostream;
//typedef basic_debugostream< wchar_t > wdebugostream;
//typedef basic_debugostream< TCHAR > tdebugostream; 

debugostream &getDebugCout();

#endif //_WIN32


#ifdef __QNX__
#define TESTING_STDOUT() (qDebug())
#elif defined(_WIN32)
#define TESTING_STDOUT() (getDebugCout())
#else
#define TESTING_STDOUT() (std::cout)
#endif //__QNX___

#ifdef _WIN32
#define TESTING_SLEEP(xMilliseconds) {::Sleep(xMilliseconds);}
#else
#define TESTING_SLEEP(xMilliseconds) {std::this_thread::sleep_for(zsLib::Milliseconds(xMilliseconds));}
#endif //_WIN32


namespace Testing
{
  std::atomic_uint &getGlobalPassedVar();
  std::atomic_uint &getGlobalFailedVar();

  void installLogger();
  void uninstallLogger();
  void passed();
  void failed();
  void output();
  void runAllTests();
}

#define TESTING_INSTALL_LOGGER()                                             \
  {Testing::installLogger();}

#define TESTING_UNINSTALL_LOGGER()                                           \
  {Testing::uninstallLogger();}

#define TESTING_AUTO_TEST_SUITE(xParam) namespace xParam {

#define TESTING_AUTO_TEST_SUITE_END() }

#define TESTING_AUTO_TEST_CASE(xTestCase) static struct Test_##xTestCase {   \
           Test_##xTestCase()                                              \
           {                                                               \
             TESTING_STDOUT() << "STARTING:     " #xTestCase "\n";                \
             try                                                           \
             { test_func(); }                                              \
             catch(...)                                                    \
             { TESTING_STDOUT() << "***UNCAUGHT EXCEPTION IN***: " #xTestCase "\n"; Testing::failed(); }   \
             std::cout << "ENDING:       " #xTestCase "\n\n";              \
           }                                                               \
           void test_func();                                               \
         } g_Test_##xTestCase;                                             \
         \
        void Test_##xTestCase::test_func()

#define TESTING_CHECK(xValue)                                         \
  if (!(xValue))                                                    \
  { TESTING_STDOUT() << "***FAILED***: " #xValue "\n"; Testing::failed(); }                   \
  else                                                              \
  { TESTING_STDOUT() << "PASSED:       " #xValue "\n"; Testing::passed(); }

#define TESTING_EQUAL(xValue1, xValue2)                               \
  if (!((xValue1) == (xValue2)))                                    \
  { std::stringstream sv1, sv2; sv1 << (xValue1); sv2 << (xValue2); TESTING_STDOUT() << "***FAILED***: " #xValue1 " == " #xValue2 ", V1=" << (sv1.str().c_str()) << ", V2=" << (sv2.str().c_str()) << "\n"; Testing::failed(); }                   \
  else                                                              \
  { std::stringstream sv1, sv2; sv1 << (xValue1); sv2 << (xValue2); TESTING_STDOUT() << "PASSED:       " #xValue1 " == " #xValue2 ", V1=" << (sv1.str().c_str()) << ", V2=" << (sv2.str().c_str()) << "\n"; Testing::passed(); }

#define TESTING_RUN_TEST_FUNC(xTestCase) \
{                                                               \
  std::cout << "STARTING:     " #xTestCase "\n";                \
  try                                                           \
  { xTestCase(); }                                              \
  catch(...)                                                    \
  { TESTING_STDOUT() << "***UNCAUGHT EXCEPTION IN***: " #xTestCase "\n"; Testing::failed(); }   \
  TESTING_STDOUT() << "ENDING:       " #xTestCase "\n\n";              \
}                                                               \

#endif //TESTING_AUTO_TEST_SUITE

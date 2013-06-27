/* Copyright 2009 MaidSafe.net limited

This MaidSafe Software is licensed under the MaidSafe.net Commercial License, version 1.0 or later,
and The General Public License (GPL), version 3. By contributing code to this project You agree to
the terms laid out in the MaidSafe Contributor Agreement, version 1.0, found in the root directory
of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also available at:

http://www.novinet.com/license

Unless required by applicable law or agreed to in writing, software distributed under the License is
distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. See the License for the specific language governing permissions and limitations under the
License.
*/

#include <cstddef>
#include "boost/detail/lightweight_test.hpp"
#include "boost/detail/atomic_count.hpp"
#include "boost/make_shared.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "maidsafe/common/test.h"

class X {
 public:
  explicit X(int a1,
             int a2,
             int a3,
             int a4,
             int a5,
             int a6,
             int a7,
             int a8,
             int a9)
      : v(a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9) {
    ++instances;
  }

  ~X() {
    --instances;
  }

  static int instances;
  int v;

 private:
  X(X const &);  // NOLINT

  X & operator=(X const &);

  void * operator new(std::size_t n) {
    // lack of this definition causes link errors on Comeau C++
    BOOST_ERROR("private X::new called");
    return ::operator new(n);
  }

  void operator delete(void * p) {
    // lack of this definition causes link errors on MSVC
    BOOST_ERROR("private X::delete called");
    ::operator delete(p);
  }
};

int X::instances = 0;

TEST(boost, BEH_BOOST_smart_ptr_AtomicCount1) {
  boost::detail::atomic_count n(4);
  ASSERT_EQ(n, 4L);
  ++n;
  ASSERT_EQ(n, 5L);
  ASSERT_NE(--n, 0L);
  boost::detail::atomic_count m(0);
  ASSERT_EQ(m, 0);
  ++m;
  ASSERT_EQ(m, 1);
  ++m;
  ASSERT_EQ(m, 2);
  ASSERT_NE(--m, 0);
  ASSERT_EQ(--m, 0);
}

TEST(boost, BEH_BOOST_smart_ptr_AtomicCount2) {
  boost::detail::atomic_count n(4);
  ASSERT_EQ(n, 4);
  ASSERT_EQ(++n, 5);
  ASSERT_EQ(++n, 6);
  ASSERT_EQ(n, 6);
  ASSERT_EQ(--n, 5);
  ASSERT_EQ(--n, 4);
  ASSERT_EQ(--n, 3);
  boost::detail::atomic_count m(0);
  ASSERT_EQ(m, 0);
  ASSERT_EQ(++m, 1);
  ASSERT_EQ(++m, 2);
  ASSERT_EQ(m, 2);
  ASSERT_EQ(--m, 1);
  ASSERT_EQ(--m, 0);
  ASSERT_EQ(--m, -1);
  ASSERT_EQ(--m, -2);
  ASSERT_EQ(--m, -3);
  ASSERT_EQ(--m, -4);
  ASSERT_EQ(++m, -3);
  ASSERT_EQ(--m, -4);
}

TEST(boost, BEH_BOOST_smart_ptr_make_shared) {
  {
    boost::shared_ptr<int> pi = boost::make_shared<int>();
    bool result = pi.get() != 0;
    ASSERT_TRUE(result);
    ASSERT_EQ(*pi, 0);
  }

  {
    boost::shared_ptr<int> pi = boost::make_shared<int>(5);
    bool result = pi.get() != 0;
    ASSERT_TRUE(result);
    ASSERT_EQ(*pi, 5);
  }
  ASSERT_EQ(X::instances, 0);
  {
    boost::shared_ptr<X> pi = boost::make_shared<X>(0, 0, 0, 0, 0, 0, 0, 0, 0);
    boost::weak_ptr<X> wp(pi);
    ASSERT_EQ(X::instances, 1);
    bool result = pi.get() != 0;
    ASSERT_TRUE(result);
    result = pi->v == 0;
    ASSERT_TRUE(result);
    pi.reset();
    ASSERT_EQ(X::instances, 0);
  }

  {
    boost::shared_ptr<X> pi = boost::make_shared<X>(1, 0, 0, 0, 0, 0, 0, 0, 0);
    boost::weak_ptr<X> wp(pi);
    ASSERT_EQ(X::instances, 1);
    bool result = pi.get() != 0;
    ASSERT_TRUE(result);
    result = pi->v == 1;
    ASSERT_TRUE(result);
    pi.reset();
    ASSERT_EQ(X::instances, 0);
  }
  {
    boost::shared_ptr<X> pi = boost::make_shared<X>(1, 2, 0, 0, 0, 0, 0, 0, 0);
    boost::weak_ptr<X> wp(pi);
    ASSERT_EQ(X::instances, 1);
    bool result = pi.get() != 0;
    ASSERT_TRUE(result);
    result = pi->v == 1 + 2;
    ASSERT_TRUE(result);
    pi.reset();
    ASSERT_EQ(X::instances, 0);
  }
  {
    boost::shared_ptr<X> pi = boost::make_shared<X>(1, 2, 3, 0, 0, 0, 0, 0, 0);
    boost::weak_ptr<X> wp(pi);
    ASSERT_EQ(X::instances, 1);
    bool result = pi.get() != 0;
    ASSERT_TRUE(result);
    ASSERT_EQ(pi->v, 1+2+3);
    pi.reset();
    ASSERT_EQ(X::instances, 0);
  }
  {
    boost::shared_ptr<X> pi = boost::make_shared<X>(1, 2, 3, 4, 0, 0, 0, 0, 0);
    boost::weak_ptr<X> wp(pi);
    ASSERT_EQ(X::instances, 1);
    bool result = pi.get() != 0;
    ASSERT_TRUE(result);
    ASSERT_EQ(pi->v, 1+2+3+4);
    pi.reset();
    ASSERT_EQ(X::instances, 0);
  }
  {
    boost::shared_ptr<X> pi = boost::make_shared<X>(1, 2, 3, 4, 5, 0, 0, 0, 0);
    boost::weak_ptr<X> wp(pi);
    ASSERT_EQ(X::instances, 1);
    bool result = pi.get() != 0;
    ASSERT_TRUE(result);
    ASSERT_EQ(pi->v, 1+2+3+4+5);
    pi.reset();
    ASSERT_EQ(X::instances, 0);
  }
  {
    boost::shared_ptr<X> pi = boost::make_shared<X>(1, 2, 3, 4, 5, 6, 0, 0, 0);
    boost::weak_ptr<X> wp(pi);
    ASSERT_EQ(X::instances, 1);
    bool result = pi.get() != 0;
    ASSERT_TRUE(result);
    ASSERT_EQ(pi->v, 1+2+3+4+5+6);
    pi.reset();
    ASSERT_EQ(X::instances, 0);
  }
  {
    boost::shared_ptr<X> pi = boost::make_shared<X>(1, 2, 3, 4, 5, 6, 7, 0, 0);
    boost::weak_ptr<X> wp(pi);
    ASSERT_EQ(X::instances, 1);
    bool result = pi.get() != 0;
    ASSERT_TRUE(result);
    ASSERT_EQ(pi->v, 1+2+3+4+5+6+7);
    pi.reset();
    ASSERT_EQ(X::instances, 0);
  }
  {
    boost::shared_ptr<X> pi = boost::make_shared<X>(1, 2, 3, 4, 5, 6, 7, 8, 0);
    boost::weak_ptr<X> wp(pi);
    ASSERT_EQ(X::instances, 1);
    bool result = pi.get() != 0;
    ASSERT_TRUE(result);
    ASSERT_EQ(pi->v, 1+2+3+4+5+6+7+8);
    pi.reset();
    ASSERT_EQ(X::instances, 0);
  }
  {
    boost::shared_ptr<X> pi = boost::make_shared<X>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    boost::weak_ptr<X> wp(pi);
    ASSERT_EQ(X::instances, 1);
    bool result = pi.get() != 0;
    ASSERT_TRUE(result);
    ASSERT_EQ(pi->v, 1+2+3+4+5+6+7+8+9);
    pi.reset();
    ASSERT_EQ(X::instances, 0);
  }
}

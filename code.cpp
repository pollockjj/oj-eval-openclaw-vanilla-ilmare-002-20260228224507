#pragma once
#ifndef SJTU_BIGINTEGER
#define SJTU_BIGINTEGER

// Integer 1:
// Implement a signed big integer class that only needs to support simple addition and subtraction

// Integer 2:
// Implement a signed big integer class that supports addition, subtraction, multiplication, and division, and overload related operators

// Do not use any header files other than the following
#include <complex>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>

// Do not use "using namespace std;"

namespace sjtu {
class int2048 {
private:
  static const int BASE = 100000;
  static const int BASE_DIGITS = 5;

  // sign_: 1 (positive), -1 (negative), 0 (zero)
  int sign_;
  std::vector<int> a_; // little-endian digits in BASE

  void trim();
  static int abs_compare(const int2048 &, const int2048 &);
  static std::vector<int> abs_add_vec(const std::vector<int> &, const std::vector<int> &);
  static std::vector<int> abs_sub_vec(const std::vector<int> &, const std::vector<int> &); // assume lhs >= rhs
  static std::vector<int> multiply_schoolbook(const std::vector<int> &, const std::vector<int> &);
  static std::vector<int> multiply_fft(const std::vector<int> &, const std::vector<int> &);
  static void fft(std::vector<std::complex<long double>> &, bool);

  static void divmod_abs(const int2048 &x, const int2048 &y, int2048 &q, int2048 &r);

public:
  // Constructors
  int2048();
  int2048(long long);
  int2048(const std::string &);
  int2048(const int2048 &);

  // The parameter types of the following functions are for reference only, you can choose to use constant references or not
  // If needed, you can add other required functions yourself
  // ===================================
  // Integer1
  // ===================================

  // Read a big integer
  void read(const std::string &);
  // Output the stored big integer, no need for newline
  void print();

  // Add a big integer
  int2048 &add(const int2048 &);
  // Return the sum of two big integers
  friend int2048 add(int2048, const int2048 &);

  // Subtract a big integer
  int2048 &minus(const int2048 &);
  // Return the difference of two big integers
  friend int2048 minus(int2048, const int2048 &);

  // ===================================
  // Integer2
  // ===================================

  int2048 operator+() const;
  int2048 operator-() const;

  int2048 &operator=(const int2048 &);

  int2048 &operator+=(const int2048 &);
  friend int2048 operator+(int2048, const int2048 &);

  int2048 &operator-=(const int2048 &);
  friend int2048 operator-(int2048, const int2048 &);

  int2048 &operator*=(const int2048 &);
  friend int2048 operator*(int2048, const int2048 &);

  int2048 &operator/=(const int2048 &);
  friend int2048 operator/(int2048, const int2048 &);

  int2048 &operator%=(const int2048 &);
  friend int2048 operator%(int2048, const int2048 &);

  friend std::istream &operator>>(std::istream &, int2048 &);
  friend std::ostream &operator<<(std::ostream &, const int2048 &);

  friend bool operator==(const int2048 &, const int2048 &);
  friend bool operator!=(const int2048 &, const int2048 &);
  friend bool operator<(const int2048 &, const int2048 &);
  friend bool operator>(const int2048 &, const int2048 &);
  friend bool operator<=(const int2048 &, const int2048 &);
  friend bool operator>=(const int2048 &, const int2048 &);
};
} // namespace sjtu

#endif


namespace sjtu {

namespace {

const int LOCAL_BASE = 100000;

std::vector<int> mul_vec_int(const std::vector<int> &a, int m) {
  if (m == 0 || a.empty())
    return {};
  std::vector<int> res;
  res.resize(a.size());
  long long carry = 0;
  for (std::size_t i = 0; i < a.size(); ++i) {
    long long cur = carry + 1ll * a[i] * m;
    res[i] = int(cur % LOCAL_BASE);
    carry = cur / LOCAL_BASE;
  }
  while (carry) {
    res.push_back(int(carry % LOCAL_BASE));
    carry /= LOCAL_BASE;
  }
  while (!res.empty() && res.back() == 0)
    res.pop_back();
  return res;
}

int cmp_vec_abs(const std::vector<int> &a, const std::vector<int> &b) {
  if (a.size() != b.size())
    return a.size() < b.size() ? -1 : 1;
  for (int i = int(a.size()) - 1; i >= 0; --i) {
    if (a[i] != b[i])
      return a[i] < b[i] ? -1 : 1;
  }
  return 0;
}

void div_vec_int_inplace(std::vector<int> &a, int d) {
  long long rem = 0;
  for (int i = int(a.size()) - 1; i >= 0; --i) {
    long long cur = a[i] + rem * LOCAL_BASE;
    a[i] = int(cur / d);
    rem = cur % d;
  }
  while (!a.empty() && a.back() == 0)
    a.pop_back();
}

} // namespace

int2048::int2048() : sign_(0) {}

int2048::int2048(long long x) : sign_(0) {
  if (x == 0)
    return;
  if (x < 0) {
    sign_ = -1;
    x = -x;
  } else {
    sign_ = 1;
  }
  while (x) {
    a_.push_back(int(x % BASE));
    x /= BASE;
  }
}

int2048::int2048(const std::string &s) : sign_(0) { read(s); }

int2048::int2048(const int2048 &other) : sign_(other.sign_), a_(other.a_) {}

void int2048::trim() {
  while (!a_.empty() && a_.back() == 0)
    a_.pop_back();
  if (a_.empty())
    sign_ = 0;
}

int int2048::abs_compare(const int2048 &lhs, const int2048 &rhs) {
  if (lhs.a_.size() != rhs.a_.size())
    return lhs.a_.size() < rhs.a_.size() ? -1 : 1;
  for (int i = int(lhs.a_.size()) - 1; i >= 0; --i) {
    if (lhs.a_[i] != rhs.a_[i])
      return lhs.a_[i] < rhs.a_[i] ? -1 : 1;
  }
  return 0;
}

std::vector<int> int2048::abs_add_vec(const std::vector<int> &lhs, const std::vector<int> &rhs) {
  std::size_t n = lhs.size() > rhs.size() ? lhs.size() : rhs.size();
  std::vector<int> res;
  res.resize(n);
  int carry = 0;
  for (std::size_t i = 0; i < n; ++i) {
    int cur = carry;
    if (i < lhs.size())
      cur += lhs[i];
    if (i < rhs.size())
      cur += rhs[i];
    if (cur >= BASE) {
      cur -= BASE;
      carry = 1;
    } else {
      carry = 0;
    }
    res[i] = cur;
  }
  if (carry)
    res.push_back(carry);
  return res;
}

std::vector<int> int2048::abs_sub_vec(const std::vector<int> &lhs, const std::vector<int> &rhs) {
  std::vector<int> res;
  res.resize(lhs.size());
  int carry = 0;
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    int cur = lhs[i] - carry - (i < rhs.size() ? rhs[i] : 0);
    if (cur < 0) {
      cur += BASE;
      carry = 1;
    } else {
      carry = 0;
    }
    res[i] = cur;
  }
  while (!res.empty() && res.back() == 0)
    res.pop_back();
  return res;
}

void int2048::fft(std::vector<std::complex<long double>> &a, bool invert) {
  int n = int(a.size());
  for (int i = 1, j = 0; i < n; ++i) {
    int bit = n >> 1;
    for (; j & bit; bit >>= 1)
      j ^= bit;
    j ^= bit;
    if (i < j) {
      std::complex<long double> tmp = a[i];
      a[i] = a[j];
      a[j] = tmp;
    }
  }

  const long double PI = 3.141592653589793238462643383279502884L;
  for (int len = 2; len <= n; len <<= 1) {
    long double ang = 2.0L * PI / len * (invert ? -1.0L : 1.0L);
    std::complex<long double> wlen = std::polar(1.0L, ang);
    for (int i = 0; i < n; i += len) {
      std::complex<long double> w(1.0L, 0.0L);
      int half = len >> 1;
      for (int j = 0; j < half; ++j) {
        std::complex<long double> u = a[i + j];
        std::complex<long double> v = a[i + j + half] * w;
        a[i + j] = u + v;
        a[i + j + half] = u - v;
        w *= wlen;
      }
    }
  }

  if (invert) {
    for (int i = 0; i < n; ++i)
      a[i] /= n;
  }
}

std::vector<int> int2048::multiply_schoolbook(const std::vector<int> &lhs, const std::vector<int> &rhs) {
  if (lhs.empty() || rhs.empty())
    return {};
  std::vector<int> res(lhs.size() + rhs.size(), 0);
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    long long carry = 0;
    for (std::size_t j = 0; j < rhs.size() || carry; ++j) {
      long long cur = res[i + j] + carry;
      if (j < rhs.size())
        cur += 1ll * lhs[i] * rhs[j];
      res[i + j] = int(cur % BASE);
      carry = cur / BASE;
    }
  }
  while (!res.empty() && res.back() == 0)
    res.pop_back();
  return res;
}

std::vector<int> int2048::multiply_fft(const std::vector<int> &lhs, const std::vector<int> &rhs) {
  if (lhs.empty() || rhs.empty())
    return {};

  std::size_t n = 1;
  while (n < lhs.size() + rhs.size())
    n <<= 1;

  std::vector<std::complex<long double>> fa(n), fb(n);
  for (std::size_t i = 0; i < lhs.size(); ++i)
    fa[i] = std::complex<long double>(lhs[i], 0.0L);
  for (std::size_t i = 0; i < rhs.size(); ++i)
    fb[i] = std::complex<long double>(rhs[i], 0.0L);

  fft(fa, false);
  fft(fb, false);
  for (std::size_t i = 0; i < n; ++i)
    fa[i] *= fb[i];
  fft(fa, true);

  std::vector<int> res(n, 0);
  long long carry = 0;
  for (std::size_t i = 0; i < n; ++i) {
    long long v = (long long)(fa[i].real() + (fa[i].real() >= 0 ? 0.5 : -0.5));
    long long cur = v + carry;
    long long digit = cur % BASE;
    if (digit < 0) {
      digit += BASE;
      cur -= BASE;
    }
    res[i] = int(digit);
    carry = cur / BASE;
  }
  while (carry) {
    int digit = int(carry % BASE);
    if (digit < 0)
      digit += BASE;
    res.push_back(digit);
    carry /= BASE;
  }

  while (!res.empty() && res.back() == 0)
    res.pop_back();
  return res;
}

void int2048::divmod_abs(const int2048 &x, const int2048 &y, int2048 &q, int2048 &r) {
  // x >= 0, y > 0
  if (x.sign_ == 0) {
    q = 0;
    r = 0;
    return;
  }
  if (abs_compare(x, y) < 0) {
    q = 0;
    r = x;
    return;
  }

  int norm = BASE / (y.a_.back() + 1);
  int2048 a = x;
  int2048 b = y;
  if (norm != 1) {
    a.a_ = mul_vec_int(a.a_, norm);
    b.a_ = mul_vec_int(b.a_, norm);
  }
  a.sign_ = a.a_.empty() ? 0 : 1;
  b.sign_ = b.a_.empty() ? 0 : 1;

  q.a_.assign(a.a_.size(), 0);
  q.sign_ = 1;

  const int n = int(a.a_.size());
  std::vector<int> rv(n + 2, 0); // little-endian remainder window buffer
  int head = n + 1;
  int len = 0;

  for (int i = n - 1; i >= 0; --i) {
    // r = r * BASE + a[i]
    if (len == 0) {
      rv[head] = a.a_[i];
      len = 1;
    } else {
      --head;
      rv[head] = a.a_[i];
      ++len;
    }
    while (len > 0 && rv[head + len - 1] == 0)
      --len;

    int bsz = int(b.a_.size());
    int s1 = (len <= bsz) ? 0 : rv[head + bsz];
    int s2 = (len <= bsz - 1) ? 0 : rv[head + bsz - 1];
    long long d = (1ll * BASE * s1 + s2) / b.a_.back();
    if (d >= BASE)
      d = BASE - 1;

    std::vector<int> bd = mul_vec_int(b.a_, int(d));

    auto cmp_r_bd = [&](const std::vector<int> &rhs) -> int {
      if (len != int(rhs.size()))
        return len < int(rhs.size()) ? -1 : 1;
      for (int k = len - 1; k >= 0; --k) {
        int lv = rv[head + k];
        int rvv = rhs[k];
        if (lv != rvv)
          return lv < rvv ? -1 : 1;
      }
      return 0;
    };

    while (cmp_r_bd(bd) < 0) {
      --d;
      bd = abs_sub_vec(bd, b.a_);
    }

    int carry = 0;
    for (int k = 0; k < int(bd.size()); ++k) {
      int cur = rv[head + k] - bd[k] - carry;
      if (cur < 0) {
        cur += BASE;
        carry = 1;
      } else {
        carry = 0;
      }
      rv[head + k] = cur;
    }
    for (int k = int(bd.size()); carry && k < len; ++k) {
      int cur = rv[head + k] - carry;
      if (cur < 0) {
        rv[head + k] = cur + BASE;
        carry = 1;
      } else {
        rv[head + k] = cur;
        carry = 0;
      }
    }

    while (len > 0 && rv[head + len - 1] == 0)
      --len;

    q.a_[i] = int(d);
  }

  q.trim();
  r.a_.clear();
  if (len > 0) {
    r.a_.reserve(len);
    for (int i = 0; i < len; ++i)
      r.a_.push_back(rv[head + i]);
  }
  if (norm != 1)
    div_vec_int_inplace(r.a_, norm);
  r.sign_ = r.a_.empty() ? 0 : 1;
}

void int2048::read(const std::string &s) {
  sign_ = 0;
  a_.clear();
  if (s.empty())
    return;

  int pos = 0;
  int sgn = 1;
  if (s[0] == '-') {
    sgn = -1;
    pos = 1;
  } else if (s[0] == '+') {
    pos = 1;
  }

  while (pos < int(s.size()) && s[pos] == '0')
    ++pos;
  if (pos >= int(s.size())) {
    sign_ = 0;
    return;
  }

  for (int i = int(s.size()); i > pos; i -= BASE_DIGITS) {
    int l = i - BASE_DIGITS;
    if (l < pos)
      l = pos;
    int cur = 0;
    for (int j = l; j < i; ++j)
      cur = cur * 10 + (s[j] - '0');
    a_.push_back(cur);
  }

  sign_ = sgn;
  trim();
}

void int2048::print() { std::cout << *this; }

int2048 &int2048::add(const int2048 &rhs) { return (*this += rhs); }

int2048 add(int2048 lhs, const int2048 &rhs) { return lhs += rhs; }

int2048 &int2048::minus(const int2048 &rhs) { return (*this -= rhs); }

int2048 minus(int2048 lhs, const int2048 &rhs) { return lhs -= rhs; }

int2048 int2048::operator+() const { return *this; }

int2048 int2048::operator-() const {
  int2048 res(*this);
  if (res.sign_ != 0)
    res.sign_ = -res.sign_;
  return res;
}

int2048 &int2048::operator=(const int2048 &rhs) {
  if (this == &rhs)
    return *this;
  sign_ = rhs.sign_;
  a_ = rhs.a_;
  return *this;
}

int2048 &int2048::operator+=(const int2048 &rhs) {
  if (rhs.sign_ == 0)
    return *this;
  if (sign_ == 0) {
    *this = rhs;
    return *this;
  }

  if (sign_ == rhs.sign_) {
    a_ = abs_add_vec(a_, rhs.a_);
  } else {
    int cmp = abs_compare(*this, rhs);
    if (cmp == 0) {
      sign_ = 0;
      a_.clear();
      return *this;
    }
    if (cmp > 0) {
      a_ = abs_sub_vec(a_, rhs.a_);
    } else {
      a_ = abs_sub_vec(rhs.a_, a_);
      sign_ = rhs.sign_;
    }
  }
  trim();
  return *this;
}

int2048 operator+(int2048 lhs, const int2048 &rhs) { return lhs += rhs; }

int2048 &int2048::operator-=(const int2048 &rhs) {
  int2048 tmp = rhs;
  if (tmp.sign_ != 0)
    tmp.sign_ = -tmp.sign_;
  return (*this += tmp);
}

int2048 operator-(int2048 lhs, const int2048 &rhs) { return lhs -= rhs; }

int2048 &int2048::operator*=(const int2048 &rhs) {
  if (sign_ == 0 || rhs.sign_ == 0) {
    sign_ = 0;
    a_.clear();
    return *this;
  }

  const std::size_t n = a_.size();
  const std::size_t m = rhs.a_.size();
  if ((n <= 64 && m <= 64) || (n + m <= 256))
    a_ = multiply_schoolbook(a_, rhs.a_);
  else
    a_ = multiply_fft(a_, rhs.a_);

  sign_ = sign_ * rhs.sign_;
  trim();
  return *this;
}

int2048 operator*(int2048 lhs, const int2048 &rhs) { return lhs *= rhs; }

int2048 &int2048::operator/=(const int2048 &rhs) {
  int2048 q = (*this) / rhs;
  *this = q;
  return *this;
}

int2048 operator/(int2048 lhs, const int2048 &rhs) {
  if (lhs.sign_ == 0)
    return lhs;

  int2048 a = lhs;
  int2048 b = rhs;
  a.sign_ = a.sign_ == 0 ? 0 : 1;
  b.sign_ = b.sign_ == 0 ? 0 : 1;

  int2048 qabs, rabs;
  int2048::divmod_abs(a, b, qabs, rabs);

  int2048 q;
  int2048 rem;

  bool same_sign = (lhs.sign_ == rhs.sign_);
  if (same_sign) {
    q = qabs;
    q.sign_ = q.a_.empty() ? 0 : 1;
    rem = rabs;
    if (!rem.a_.empty())
      rem.sign_ = rhs.sign_;
  } else {
    if (rabs.sign_ == 0) {
      q = qabs;
      if (q.sign_ != 0)
        q.sign_ = -1;
      rem = 0;
    } else {
      q = qabs;
      q += int2048(1);
      if (q.sign_ != 0)
        q.sign_ = -1;

      int2048 absb = b;
      rem.a_ = int2048::abs_sub_vec(absb.a_, rabs.a_);
      rem.sign_ = rem.a_.empty() ? 0 : rhs.sign_;
    }
  }

  q.trim();
  return q;
}

int2048 &int2048::operator%=(const int2048 &rhs) {
  int2048 r = (*this) % rhs;
  *this = r;
  return *this;
}

int2048 operator%(int2048 lhs, const int2048 &rhs) {
  int2048 q = lhs / rhs;
  lhs -= q * rhs;
  lhs.trim();
  return lhs;
}

std::istream &operator>>(std::istream &is, int2048 &x) {
  std::string s;
  is >> s;
  x.read(s);
  return is;
}

std::ostream &operator<<(std::ostream &os, const int2048 &x) {
  if (x.sign_ == 0) {
    os << '0';
    return os;
  }
  if (x.sign_ < 0)
    os << '-';

  int i = int(x.a_.size()) - 1;
  os << x.a_[i];
  for (--i; i >= 0; --i) {
    int d = x.a_[i];
    int div = int2048::BASE / 10;
    while (div > 0) {
      os << char('0' + (d / div));
      d %= div;
      div /= 10;
    }
  }
  return os;
}

bool operator==(const int2048 &lhs, const int2048 &rhs) {
  if (lhs.sign_ != rhs.sign_)
    return false;
  if (lhs.a_.size() != rhs.a_.size())
    return false;
  for (std::size_t i = 0; i < lhs.a_.size(); ++i) {
    if (lhs.a_[i] != rhs.a_[i])
      return false;
  }
  return true;
}

bool operator!=(const int2048 &lhs, const int2048 &rhs) { return !(lhs == rhs); }

bool operator<(const int2048 &lhs, const int2048 &rhs) {
  if (lhs.sign_ != rhs.sign_)
    return lhs.sign_ < rhs.sign_;
  if (lhs.sign_ == 0)
    return false;
  int cmp = int2048::abs_compare(lhs, rhs);
  if (lhs.sign_ > 0)
    return cmp < 0;
  return cmp > 0;
}

bool operator>(const int2048 &lhs, const int2048 &rhs) { return rhs < lhs; }

bool operator<=(const int2048 &lhs, const int2048 &rhs) { return !(rhs < lhs); }

bool operator>=(const int2048 &lhs, const int2048 &rhs) { return !(lhs < rhs); }

} // namespace sjtu

// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_CASTABLE_H_
#define ELANG_BASE_CASTABLE_H_

#include <type_traits>

namespace elang {

// TODO(eval1749) Once our tool chain supports C++14 |std::is_final|, we should
// replace |elang::is_final<T>| to |std::is_final<T>|.
template <typename T>
struct is_final : std::false_type {};

// T* as()
// const char* class_name()
// bool is<T>()
// static const char* static_class_name()
template <typename Base>
class Castable {
 public:
  template <class Class>
  Class* as() {
    return is<Class>() ? static_cast<Class*>(this) : nullptr;
  }
  template <class Class>
  const Class* as() const {
    return is<Class>() ? static_cast<const Class*>(this) : nullptr;
  }
  virtual const char* class_name() const { return static_class_name(); }
  template <class Class>
  bool is() const {
    static_assert(std::is_base_of<Base, Class>::value, "Unrelated types");
    // TODO(eval1749) We should use C++14 |std::is_final<T>|.
    if (is_final<Class>::value)
      return this && class_name() == Class::static_class_name();
    return this && is_class_of(Class::static_class_name());
  }
  virtual bool is_class_of(const char* other_name) const {
    return static_class_name() == other_name;
  }
  static const char* static_class_name() { return "Castable"; }

 protected:
  virtual ~Castable() = default;
};

#define DECLARE_CASTABLE_CLASS(this_name, base_name)                      \
 public:                                                                  \
  static const char* static_class_name() { return #this_name; }           \
  const char* class_name() const override { return static_class_name(); } \
  bool is_class_of(const char* other_name) const override {               \
    return static_class_name() == other_name ||                           \
           base_name::is_class_of(other_name);                            \
  }                                                                       \
                                                                          \
 private:
}  // namespace elang

#endif  // ELANG_BASE_CASTABLE_H_

// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/analyze/name_resolver.h"

#include "base/logging.h"
#include "elang/compiler/analyze/analyze_factory.h"
#include "elang/compiler/analyze/predefined_types.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/node_factory.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/token_data.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

#define FOR_EACH_KEYWORD_TYPE(V) \
  V(Bool)                        \
  V(Char)                        \
  V(Float32)                     \
  V(Float64)                     \
  V(Int8)                        \
  V(Int16)                       \
  V(Int32)                       \
  V(Int64)                       \
  V(UInt8)                       \
  V(UInt16)                      \
  V(UInt32)                      \
  V(UInt64)                      \
  V(Void)

namespace {
ast::NamespaceMember* GetMember(CompilationSession* session,
                                ast::MemberContainer* container,
                                base::StringPiece16 name) {
  auto const member = container->FindMember(session->NewAtomicString(name));
  DCHECK(member);
  return member;
}

ast::Class* GetClass(CompilationSession* session,
                     ast::MemberContainer* container,
                     base::StringPiece16 name) {
  auto const member = container->FindMember(session->NewAtomicString(name));
  DCHECK(member);
  return member->as<ast::Class>();
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// NameResolver
//
NameResolver::NameResolver(CompilationSession* session)
    : factory_(new AnalyzeFactory()),
      predefined_types_(new PredefinedTypes(session)),
      session_(session) {
#define V(Name) \
  keyword_types_[TokenType::Name] = type_from(PredefinedName::Name);
  FOR_EACH_KEYWORD_TYPE(V)
#undef V
  keyword_types_[TokenType::Int] = type_from(PredefinedName::Int32);
}

NameResolver::~NameResolver() {
}

ast::Class* NameResolver::type_from(PredefinedName name) const {
  return predefined_types_->type_from(name);
}

void NameResolver::DidResolveReference(ast::Expression* reference,
                                       ast::NamespaceMember* member) {
  DCHECK(member);
  DCHECK(!map_.count(reference));
  map_[reference] = member;
}

ast::NamespaceMember* NameResolver::FindReference(ast::Expression* reference) {
  auto const it = map_.find(reference);
  if (it != map_.end())
    return it->second;
  if (reference->is<ast::NameReference>()) {
    auto const it = keyword_types_.find(reference->token()->type());
    if (it != keyword_types_.end())
      return it->second;
  }
  return nullptr;
}

}  // namespace compiler
}  // namespace elang

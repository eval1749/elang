// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_API_MACHINE_CODE_BUILDER_H_
#define ELANG_API_MACHINE_CODE_BUILDER_H_

#include <string>

#include "base/basictypes.h"
#include "base/strings/string_piece.h"
#include "elang/api/source_code_location.h"
#include "elang/base/float_types.h"

namespace elang {
namespace api {

//////////////////////////////////////////////////////////////////////
//
// MachineCodeBuilder
//
class MachineCodeBuilder {
 public:
  MachineCodeBuilder();
  virtual ~MachineCodeBuilder();

  virtual void EmitCode(const uint8_t* codes, size_t code_size) = 0;
  virtual void FinishCode() = 0;
  virtual void PrepareCode(size_t code_size) = 0;
  virtual void SetCallSite(size_t offset, base::StringPiece16 string) = 0;
  virtual void SetCodeOffset(size_t offset, size_t target_offset) = 0;
  virtual void SetFloat32(size_t offset, float32_t float32) = 0;
  virtual void SetFloat64(size_t offset, float64_t float64) = 0;
  virtual void SetInt32(size_t offset, int32_t int32) = 0;
  virtual void SetInt64(size_t offset, int64_t int64) = 0;
  virtual void SetSourceCodeLocation(size_t offset,
                                     SourceCodeLocation location) = 0;
  virtual void SetString(size_t offset, base::StringPiece16 string) = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(MachineCodeBuilder);
};

}  // namespace api
}  // namespace elang

#endif  // ELANG_API_MACHINE_CODE_BUILDER_H_

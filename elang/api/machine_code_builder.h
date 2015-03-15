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

  virtual void EmitCode(const uint8_t* codes, int code_size) = 0;
  virtual void FinishCode() = 0;
  virtual void PrepareCode(int code_size) = 0;
  virtual void SetCallSite(int offset, base::StringPiece16 string) = 0;
  virtual void SetCodeOffset(int offset, int target_offset) = 0;
  virtual void SetFloat32(int offset, float32_t float32) = 0;
  virtual void SetFloat64(int offset, float64_t float64) = 0;
  virtual void SetInt32(int offset, int32_t int32) = 0;
  virtual void SetInt64(int offset, int64_t int64) = 0;
  virtual void SetSourceCodeLocation(int offset,
                                     SourceCodeLocation location) = 0;
  virtual void SetString(int offset, base::StringPiece16 string) = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(MachineCodeBuilder);
};

}  // namespace api
}  // namespace elang

#endif  // ELANG_API_MACHINE_CODE_BUILDER_H_

// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_SHELL_SOURCE_FILE_STREAM_H_
#define ELANG_SHELL_SOURCE_FILE_STREAM_H_

#include <memory>
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/files/file.h"
#include "base/files/file_path.h"
#include "elang/compiler/character_stream.h"

namespace elang {
namespace compiler {
namespace shell {

class Utf8Decoder;

//////////////////////////////////////////////////////////////////////
//
// SourceFileStream
//
class SourceFileStream final : public CharacterStream {
 public:
  explicit SourceFileStream(const base::FilePath& file_path);
  ~SourceFileStream() final;

  std::string error_details() const;
  const base::File& file() const { return file_; }
  const base::FilePath& file_path() const { return file_path_; }

 private:
  bool Fill();

  // compiler::CharacterStream
  bool IsAtEndOfStream() final;
  base::char16 ReadChar() final;

  base::File file_;
  base::FilePath file_path_;
  std::vector<char> utf8_buffer_;
  std::unique_ptr<Utf8Decoder> utf8_decoder_;
  size_t utf8_position_;

  DISALLOW_COPY_AND_ASSIGN(SourceFileStream);
};

}  // namespace shell
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_SHELL_SOURCE_FILE_STREAM_H_

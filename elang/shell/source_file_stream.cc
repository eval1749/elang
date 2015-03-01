// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/shell/source_file_stream.h"

#include "base/logging.h"
#include "elang/shell/utf8_decoder.h"

namespace elang {
namespace compiler {
namespace shell {

static size_t kBufferSize = 1024 * 16;

//////////////////////////////////////////////////////////////////////
//
// SourceFileStream
//
SourceFileStream::SourceFileStream(const base::FilePath& file_path)
    : file_(file_path, base::File::FLAG_OPEN | base::File::FLAG_READ),
      file_path_(file_path),
      utf8_buffer_(kBufferSize),
      utf8_decoder_(new Utf8Decoder()),
      utf8_position_(0) {
  utf8_buffer_.resize(0);
}

SourceFileStream::~SourceFileStream() {
}

std::string SourceFileStream::error_details() const {
  return base::File::ErrorToString(file_.error_details());
}

bool SourceFileStream::Fill() {
  DCHECK(!utf8_position_);
  if (!file_.IsValid())
    return false;
  auto const buffer_size = static_cast<int>(utf8_buffer_.capacity());
  utf8_buffer_.resize(buffer_size);
  auto const read = file_.ReadAtCurrentPos(utf8_buffer_.data(), buffer_size);
  if (read <= 0) {
    file_.Close();
    utf8_buffer_.clear();
    return false;
  }
  utf8_buffer_.resize(read);
  return true;
}

// compiler::CharacterStream
bool SourceFileStream::IsAtEndOfStream() {
  if (!utf8_decoder_->IsValid())
    return true;
  if (utf8_position_ < utf8_buffer_.size())
    return false;
  utf8_position_ = 0;
  return !Fill();
}

base::char16 SourceFileStream::ReadChar() {
  while (utf8_decoder_->IsValid()) {
    if (utf8_decoder_->HasChar())
      return utf8_decoder_->Get();
    if (IsAtEndOfStream())
      return static_cast<base::char16>(-1);
    DCHECK_LT(utf8_position_, utf8_buffer_.size());
    utf8_decoder_->Feed(utf8_buffer_[utf8_position_]);
    ++utf8_position_;
  }
  return static_cast<base::char16>(-1);
}

}  // namespace shell
}  // namespace compiler
}  // namespace elang

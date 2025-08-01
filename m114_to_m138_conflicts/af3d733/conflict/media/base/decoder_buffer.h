// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_BASE_DECODER_BUFFER_H_
#define MEDIA_BASE_DECODER_BUFFER_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <utility>

#include "base/check.h"
#include "base/containers/heap_array.h"
#include "base/containers/span.h"
#include "base/memory/raw_span.h"
#include "base/memory/read_only_shared_memory_region.h"
#include "base/memory/ref_counted.h"
#include "base/memory/shared_memory_mapping.h"
#include "base/memory/unsafe_shared_memory_region.h"
#include "base/time/time.h"
#include "base/types/pass_key.h"
#include "build/build_config.h"
#include "media/base/decoder_buffer_side_data.h"
#include "media/base/decrypt_config.h"
#include "media/base/media_export.h"
#include "media/base/timestamp_constants.h"

#if BUILDFLAG(USE_STARBOARD_MEDIA)
#include "starboard/media.h"
#endif // BUILDFLAG(USE_STARBOARD_MEDIA)

namespace media {

// A specialized buffer for interfacing with audio / video decoders.
//
// Also includes decoder specific functionality for decryption.
//
// NOTE: It is illegal to call any method when end_of_stream() is true.
class MEDIA_EXPORT DecoderBuffer
    : public base::RefCountedThreadSafe<DecoderBuffer> {
 public:
  REQUIRE_ADOPTION_FOR_REFCOUNTED_TYPE();

  // ExternalMemory wraps a class owning a buffer and expose the data interface
  // through Span(). This class is derived by a class that owns the class owning
  // the buffer owner class.
  struct MEDIA_EXPORT ExternalMemory {
   public:
    virtual ~ExternalMemory() = default;
    virtual const base::span<const uint8_t> Span() const = 0;
  };

  using DiscardPadding = DecoderBufferSideData::DiscardPadding;

  // TODO(crbug.com/365814210): Remove this structure. It's barely used outside
  // of unit tests.
  struct MEDIA_EXPORT TimeInfo {
    // Presentation time of the frame.
    base::TimeDelta timestamp;

    // Presentation duration of the frame.
    base::TimeDelta duration;

    // Duration of (audio) samples from the beginning and end of this frame
    // which should be discarded after decoding. A value of kInfiniteDuration
    // for the first value indicates the entire frame should be discarded; the
    // second value must be base::TimeDelta() in this case.
    DiscardPadding discard_padding;
  };

<<<<<<< HEAD
  // Allocates buffer with |size| > 0. |is_key_frame_| will default to false.
  // If size is 0, no buffer will be allocated.
  // TODO(crbug.com/365814210): Remove this constructor. Clients should use the
  // FromArray constructor instead asking for a writable DecoderBuffer.
=======
#if BUILDFLAG(USE_STARBOARD_MEDIA)
  class Allocator {
   public:
    static Allocator* GetInstance();

    // The function should never return nullptr.  It may terminate the app on
    // allocation failure.
    virtual void* Allocate(size_t size, size_t alignment) = 0;
    virtual void Free(void* p, size_t size) = 0;

    virtual int GetAudioBufferBudget() const = 0;
    virtual int GetBufferAlignment() const = 0;
    virtual int GetBufferPadding() const = 0;
    virtual base::TimeDelta GetBufferGarbageCollectionDurationThreshold() const = 0;
    virtual int GetProgressiveBufferBudget(SbMediaVideoCodec codec,
                                           int resolution_width,
                                           int resolution_height,
                                           int bits_per_pixel) const = 0;
    virtual int GetVideoBufferBudget(SbMediaVideoCodec codec,
                                     int resolution_width,
                                     int resolution_height,
                                     int bits_per_pixel) const = 0;

   protected:
    ~Allocator() {}

    static void Set(Allocator* allocator);
  };
#endif // BUILDFLAG(USE_STARBOARD_MEDIA)

  // Allocates buffer with |size| >= 0. |is_key_frame_| will default to false.
>>>>>>> af3d7334d8c ([media] Support DecoderBufferAllocator (#4348))
  explicit DecoderBuffer(size_t size);

  // Allocates a buffer with a copy of `data` in it. `is_key_frame_` will
  // default to false.
  DecoderBuffer(base::PassKey<DecoderBuffer>, base::span<const uint8_t> data);
  DecoderBuffer(base::PassKey<DecoderBuffer>, base::HeapArray<uint8_t> data);
  DecoderBuffer(base::PassKey<DecoderBuffer>,
                std::unique_ptr<ExternalMemory> external_memory);
  enum class DecoderBufferType { kNormal, kEndOfStream };
  using ConfigVariant = DecoderBufferSideData::ConfigVariant;
  DecoderBuffer(base::PassKey<DecoderBuffer>,
                DecoderBufferType decoder_buffer_type,
                std::optional<ConfigVariant> next_config);
  DecoderBuffer(const DecoderBuffer&) = delete;
  DecoderBuffer& operator=(const DecoderBuffer&) = delete;

  // Create a DecoderBuffer whose |data_| is copied from |data|. The buffer's
  // |is_key_frame_| will default to false.
  static scoped_refptr<DecoderBuffer> CopyFrom(base::span<const uint8_t> data);

  // Create a DecoderBuffer where data() of |size| bytes resides within the heap
  // as byte array. The buffer's |is_key_frame_| will default to false.
  //
  // Ownership of |data| is transferred to the buffer.
  static scoped_refptr<DecoderBuffer> FromArray(base::HeapArray<uint8_t> data);

  // Create a DecoderBuffer where data() of |size| bytes resides within the
  // memory referred to by |region| at non-negative offset |offset|. The
  // buffer's |is_key_frame_| will default to false.
  //
  // The shared memory will be mapped read-only.
  //
  // If mapping fails, nullptr will be returned.
  static scoped_refptr<DecoderBuffer> FromSharedMemoryRegion(
      base::UnsafeSharedMemoryRegion region,
      uint64_t offset,
      size_t size);

  // Create a DecoderBuffer where data() of |size| bytes resides within the
  // ReadOnlySharedMemoryRegion referred to by |mapping| at non-negative offset
  // |offset|. The buffer's |is_key_frame_| will default to false.
  //
  // Ownership of |region| is transferred to the buffer.
  static scoped_refptr<DecoderBuffer> FromSharedMemoryRegion(
      base::ReadOnlySharedMemoryRegion region,
      uint64_t offset,
      size_t size);

  // Creates a DecoderBuffer with ExternalMemory. The buffer accessed through
  // the created DecoderBuffer is |span| of |external_memory||.
  // |external_memory| is owned by DecoderBuffer until it is destroyed.
  static scoped_refptr<DecoderBuffer> FromExternalMemory(
      std::unique_ptr<ExternalMemory> external_memory);

  // Create a DecoderBuffer indicating we've reached end of stream. If this is
  // an EOS buffer for a config change, the upcoming config may optionally be
  // provided to allow the decoder to make more optimal configuration decisions.
  //
  // Calling any method other than end_of_stream() or next_config() on the
  // resulting buffer is disallowed.
  static scoped_refptr<DecoderBuffer> CreateEOSBuffer(
      std::optional<ConfigVariant> next_config = std::nullopt);

  // Method to verify if subsamples of a DecoderBuffer match.
  static bool DoSubsamplesMatch(const DecoderBuffer& buffer);

  // TODO(crbug.com/365814210): Remove this method.
  TimeInfo time_info() const {
    DCHECK(!end_of_stream());
    return {timestamp_, duration_, discard_padding()};
  }

  base::TimeDelta timestamp() const {
    DCHECK(!end_of_stream());
    return timestamp_;
  }

  // TODO(crbug.com/365814210): This should be renamed at some point, but to
  // avoid a yak shave keep as a virtual with hacker_style() for now.
  virtual void set_timestamp(base::TimeDelta timestamp);

  base::TimeDelta duration() const {
    DCHECK(!end_of_stream());
    return duration_;
  }

  void set_duration(base::TimeDelta duration) {
    DCHECK(!end_of_stream());
    DCHECK(duration == kNoTimestamp ||
           (duration >= base::TimeDelta() && duration != kInfiniteDuration))
        << duration.InSecondsF();
    duration_ = duration;
  }

  // The pointer to the start of the buffer. Prefer to construct a span around
  // the buffer, such as `base::span(decoder_buffer)`.
  // TODO(crbug.com/365814210): Remove in favor of AsSpan().
  const uint8_t* data() const {
    DCHECK(!end_of_stream());
<<<<<<< HEAD
    if (external_memory_)
      return external_memory_->Span().data();
    return data_.data();
=======
#if BUILDFLAG(USE_STARBOARD_MEDIA)
    return data_;
#else // BUILDFLAG(USE_STARBOARD_MEDIA)
    if (read_only_mapping_.IsValid())
      return read_only_mapping_.GetMemoryAs<const uint8_t>();
    if (writable_mapping_.IsValid())
      return writable_mapping_.GetMemoryAs<const uint8_t>();
    if (external_memory_)
      return external_memory_->span().data();
    return data_.get();
#endif // BUILDFLAG(USE_STARBOARD_MEDIA)
>>>>>>> af3d7334d8c ([media] Support DecoderBufferAllocator (#4348))
  }

  // The number of bytes in the buffer.
  size_t size() const {
    DCHECK(!end_of_stream());
    return external_memory_ ? external_memory_->Span().size() : data_.size();
  }

  // Prefer writable_span(), though it should also be removed.
  //
  // TODO(crbug.com/41383992): Remove writable_data().
  uint8_t* writable_data() const {
#if BUILDFLAG(USE_STARBOARD_MEDIA)
    return data_;
#else // BUILDFLAG(USE_STARBOARD_MEDIA)
    DCHECK(!end_of_stream());
    DCHECK(!external_memory_);
<<<<<<< HEAD
    return const_cast<uint8_t*>(data_.data());
=======
    return data_.get();
#endif // BUILDFLAG(USE_STARBOARD_MEDIA)
>>>>>>> af3d7334d8c ([media] Support DecoderBufferAllocator (#4348))
  }

  // TODO(crbug.com/41383992): Remove writable_span().
  base::span<uint8_t> writable_span() const {
    // TODO(crbug.com/40284755): `data_` should be converted to HeapArray, then
    // it can give out a span safely.
    return UNSAFE_TODO(base::span(writable_data(), size()));
  }

  bool empty() const {
    return external_memory_ ? external_memory_->Span().empty() : data_.empty();
  }

  // Read-only iteration as bytes. This allows this type to meet the
  // requirements of `std::ranges::contiguous_range`, and thus be implicitly
  // convertible to a span.
  auto begin() const {
    return external_memory_ ? external_memory_->Span().begin() : data_.begin();
  }
  auto end() const {
    return external_memory_ ? external_memory_->Span().end() : data_.end();
  }
  auto first(size_t count) const {
    return external_memory_ ? external_memory_->Span().first(count)
                            : data_.first(count);
  }
  auto subspan(size_t offset, size_t count) const {
    return external_memory_ ? external_memory_->Span().subspan(offset, count)
                            : data_.subspan(offset, count);
  }

  // TODO(crbug.com/365814210): Change the return type to std::optional.
  DiscardPadding discard_padding() const {
    DCHECK(!end_of_stream());
    return side_data_ ? side_data_->discard_padding : DiscardPadding();
  }

  // TODO(crbug.com/365814210): Remove this method and force callers to get it
  // through side_data().
  void set_discard_padding(const DiscardPadding& discard_padding);

  // Returns DecryptConfig associated with |this|. Returns null if |this| is
  // not encrypted.
  const DecryptConfig* decrypt_config() const {
    DCHECK(!end_of_stream());
    return decrypt_config_.get();
  }

  // TODO(b/331652782): integrate the setter function into the constructor to
  // make |decrypt_config_| immutable.
  void set_decrypt_config(std::unique_ptr<DecryptConfig> decrypt_config) {
    DCHECK(!end_of_stream());
    decrypt_config_ = std::move(decrypt_config);
  }

<<<<<<< HEAD
  bool end_of_stream() const { return is_end_of_stream_; }
=======
  // If there's no data in this buffer, it represents end of stream.
#if BUILDFLAG(USE_STARBOARD_MEDIA)
  bool end_of_stream() const { return !data_; }
#else // BUILDFLAG(USE_STARBOARD_MEDIA)
  bool end_of_stream() const {
    return !read_only_mapping_.IsValid() && !writable_mapping_.IsValid() &&
           !external_memory_ && !data_;
  }
#endif // BUILDFLAG(USE_STARBOARD_MEDIA)
>>>>>>> af3d7334d8c ([media] Support DecoderBufferAllocator (#4348))

  bool is_key_frame() const {
    DCHECK(!end_of_stream());
    return is_key_frame_;
  }

  bool is_encrypted() const {
    DCHECK(!end_of_stream());
    return decrypt_config() && decrypt_config()->encryption_scheme() !=
                                   EncryptionScheme::kUnencrypted;
  }

  void set_is_key_frame(bool is_key_frame) {
    DCHECK(!end_of_stream());
    is_key_frame_ = is_key_frame;
  }

  // Returns DecoderBufferSideData associated with `this`. Check if `side_data_`
  // exists using `has_side_data()` before calling this function.
  const DecoderBufferSideData* side_data() const {
    DCHECK(!end_of_stream());
    return side_data_.get();
  }

  // TODO(b/331652782): integrate the setter function into the constructor to
  // make |side_data_| immutable.
  DecoderBufferSideData& WritableSideData();
  void set_side_data(std::unique_ptr<DecoderBufferSideData> side_data);

  // Returns true if all fields in |buffer| matches this buffer including
  // |data_|.
  bool MatchesForTesting(const DecoderBuffer& buffer) const;

  // As above, except that |data_| is not compared.
  bool MatchesMetadataForTesting(const DecoderBuffer& buffer) const;

  // Returns a human-readable string describing |*this|.
  std::string AsHumanReadableString(bool verbose = false) const;

  // Returns total memory usage for both bookkeeping and buffered data. The
  // function is added for more accurately memory management.
  virtual size_t GetMemoryUsage() const;

  // Accessor for DecoderBufferSideData::next_config.
  std::optional<ConfigVariant> next_config() const {
    DCHECK(end_of_stream());
    return side_data_ ? side_data_->next_config : std::nullopt;
  }

 protected:
  friend class base::RefCountedThreadSafe<DecoderBuffer>;
  virtual ~DecoderBuffer();

<<<<<<< HEAD
  // Allocates a buffer with a copy of `data` in it. `is_key_frame_` will
  // default to false.
  explicit DecoderBuffer(base::span<const uint8_t> data);
  explicit DecoderBuffer(base::HeapArray<uint8_t> data);
  explicit DecoderBuffer(std::unique_ptr<ExternalMemory> external_memory);
  DecoderBuffer(DecoderBufferType decoder_buffer_type,
                std::optional<ConfigVariant> next_config);

  // Encoded data, if it is stored on the heap.
  const base::HeapArray<uint8_t> data_;
=======
#if BUILDFLAG(USE_STARBOARD_MEDIA)
  // Encoded data, allocated from DecoderBuffer::Allocator.
  uint8_t* data_ = nullptr;
  size_t allocated_size_ = 0;
#else // BUILDFLAG(USE_STARBOARD_MEDIA)
  // Encoded data, if it is stored on the heap.
  std::unique_ptr<uint8_t[]> data_;
#endif // BUILDFLAG(USE_STARBOARD_MEDIA)
>>>>>>> af3d7334d8c ([media] Support DecoderBufferAllocator (#4348))

 private:
  // ***************************************************************************
  // WARNING: This is a highly allocated object. Care should be taken when
  // adding any fields to make sure they are absolutely necessary. If a field
  // must be added and can be optional, ensure it is heap allocated through the
  // usage of something like std::unique_ptr.
  // ***************************************************************************

  // Presentation time of the frame.
  base::TimeDelta timestamp_;

  // Presentation duration of the frame.
  base::TimeDelta duration_;

  // Structured side data.
  std::unique_ptr<DecoderBufferSideData> side_data_;

  const std::unique_ptr<ExternalMemory> external_memory_;

  // Encryption parameters for the encoded data.
  std::unique_ptr<DecryptConfig> decrypt_config_;

  // Whether the frame was marked as a keyframe in the container.
  bool is_key_frame_ : 1 = false;

  // Whether the buffer represent the end of stream.
  const bool is_end_of_stream_ : 1 = false;
};

}  // namespace media

#endif  // MEDIA_BASE_DECODER_BUFFER_H_

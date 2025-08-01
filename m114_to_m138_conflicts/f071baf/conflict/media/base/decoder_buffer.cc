// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/base/decoder_buffer.h"

#include <sstream>

#include "base/containers/heap_array.h"
#include "base/debug/alias.h"
#include "base/memory/scoped_refptr.h"
#include "base/strings/stringprintf.h"
#include "base/types/pass_key.h"
#include "media/base/subsample_entry.h"

namespace media {

<<<<<<< HEAD
namespace {

template <class T>
class ExternalSharedMemoryAdapter : public DecoderBuffer::ExternalMemory {
 public:
  explicit ExternalSharedMemoryAdapter(T mapping)
      : mapping_(std::move(mapping)) {}

  const base::span<const uint8_t> Span() const override {
    return mapping_.template GetMemoryAsSpan<const uint8_t>();
  }

 private:
  T mapping_;
};

}  // namespace
=======
#if BUILDFLAG(USE_STARBOARD_MEDIA)
namespace {
DecoderBuffer::Allocator* s_allocator = nullptr;
}  // namespace

// static
DecoderBuffer::Allocator* DecoderBuffer::Allocator::GetInstance() {
  DCHECK(s_allocator);
  return s_allocator;
}

// static
void DecoderBuffer::Allocator::Set(Allocator* allocator) {
  s_allocator = allocator;
}
#endif // BUILDFLAG(USE_STARBOARD_MEDIA)

DecoderBuffer::TimeInfo::TimeInfo() = default;
DecoderBuffer::TimeInfo::~TimeInfo() = default;
DecoderBuffer::TimeInfo::TimeInfo(const TimeInfo&) = default;
DecoderBuffer::TimeInfo& DecoderBuffer::TimeInfo::operator=(const TimeInfo&) =
    default;
>>>>>>> af3d7334d8c ([media] Support DecoderBufferAllocator (#4348))

DecoderBuffer::DecoderBuffer(size_t size)
    : data_(base::HeapArray<uint8_t>::Uninit(size)) {}

DecoderBuffer::DecoderBuffer(base::span<const uint8_t> data)
    : data_(base::HeapArray<uint8_t>::CopiedFrom(data)) {}

<<<<<<< HEAD
DecoderBuffer::DecoderBuffer(base::HeapArray<uint8_t> data)
    : data_(std::move(data)) {}
=======
  Initialize();

#if BUILDFLAG(USE_STARBOARD_MEDIA)
  memcpy(data_, data, size_);
#else // BUILDFLAG(USE_STARBOARD_MEDIA)
  memcpy(data_.get(), data, size_);
#endif // BUILDFLAG(USE_STARBOARD_MEDIA)

  if (!side_data) {
    CHECK_EQ(side_data_size, 0u);
    return;
  }

  DCHECK_GT(side_data_size_, 0u);
  memcpy(side_data_.get(), side_data, side_data_size_);
}

#if BUILDFLAG(USE_STARBOARD_MEDIA)
DecoderBuffer::DecoderBuffer(DemuxerStream::Type type,
                             const uint8_t* data,
                             size_t size,
                             const uint8_t* side_data,
                             size_t side_data_size)
    : size_(size), side_data_size_(side_data_size), is_key_frame_(false) {
  if (!data) {
    CHECK_EQ(size_, 0u);
    CHECK(!side_data);
    return;
  }

  Initialize(type);

  memcpy(data_, data, size_);

  if (!side_data) {
    CHECK_EQ(side_data_size, 0u);
    return;
  }

  DCHECK_GT(side_data_size_, 0u);
  memcpy(side_data_.get(), side_data, side_data_size_);
}
#endif // BUILDFLAG(USE_STARBOARD_MEDIA)

DecoderBuffer::DecoderBuffer(std::unique_ptr<uint8_t[]> data, size_t size)
#if BUILDFLAG(USE_STARBOARD_MEDIA)
  : DecoderBuffer(data.get(), size, nullptr, 0) {
  // TODO(b/378106931): revisit DecoderBufferAllocator once rebase to m126+
}
#else // BUILDFLAG(USE_STARBOARD_MEDIA)
    : data_(std::move(data)), size_(size) {}
#endif // BUILDFLAG(USE_STARBOARD_MEDIA)

DecoderBuffer::DecoderBuffer(base::ReadOnlySharedMemoryMapping mapping,
                             size_t size)
    : size_(size), read_only_mapping_(std::move(mapping)) {}

DecoderBuffer::DecoderBuffer(base::WritableSharedMemoryMapping mapping,
                             size_t size)
    : size_(size), writable_mapping_(std::move(mapping)) {}
>>>>>>> af3d7334d8c ([media] Support DecoderBufferAllocator (#4348))

DecoderBuffer::DecoderBuffer(std::unique_ptr<ExternalMemory> external_memory)
    : external_memory_(std::move(external_memory)) {}

<<<<<<< HEAD
DecoderBuffer::DecoderBuffer(DecoderBufferType decoder_buffer_type,
                             std::optional<ConfigVariant> next_config)
    : is_end_of_stream_(decoder_buffer_type ==
                        DecoderBufferType::kEndOfStream) {
  if (next_config) {
    DCHECK(end_of_stream());
    side_data_ = std::make_unique<DecoderBufferSideData>();
    side_data_->next_config = std::move(next_config);
  }
}

DecoderBuffer::DecoderBuffer(base::PassKey<DecoderBuffer>,
                             base::span<const uint8_t> data)
    : DecoderBuffer(std::move(data)) {}

DecoderBuffer::DecoderBuffer(base::PassKey<DecoderBuffer>,
                             base::HeapArray<uint8_t> data)
    : DecoderBuffer(std::move(data)) {}

DecoderBuffer::DecoderBuffer(base::PassKey<DecoderBuffer>,
                             std::unique_ptr<ExternalMemory> external_memory)
    : DecoderBuffer(std::move(external_memory)) {}

DecoderBuffer::DecoderBuffer(base::PassKey<DecoderBuffer>,
                             DecoderBufferType decoder_buffer_type,
                             std::optional<ConfigVariant> next_config)
    : DecoderBuffer(decoder_buffer_type, std::move(next_config)) {}

DecoderBuffer::~DecoderBuffer() = default;
=======
DecoderBuffer::~DecoderBuffer() {
#if BUILDFLAG(USE_STARBOARD_MEDIA)
  DCHECK(s_allocator);
  s_allocator->Free(data_, allocated_size_);
#else // BUILDFLAG(USE_STARBOARD_MEDIA)
  data_.reset();
#endif // BUILDFLAG(USE_STARBOARD_MEDIA)
  side_data_.reset();
}

void DecoderBuffer::Initialize() {
#if BUILDFLAG(USE_STARBOARD_MEDIA)
  // This is used by Mojo.
  // TODO: b/369245553 - Cobalt: Investigate the memory and performance impact
  //                     of using Mojo.
  Initialize(DemuxerStream::UNKNOWN);
#else // BUILDFLAG(USE_STARBOARD_MEDIA)
  data_.reset(new uint8_t[size_]);
  if (side_data_size_ > 0)
    side_data_.reset(new uint8_t[side_data_size_]);
#endif // BUILDFLAG(USE_STARBOARD_MEDIA)
}

#if BUILDFLAG(USE_STARBOARD_MEDIA)
void DecoderBuffer::Initialize(DemuxerStream::Type type) {
  DCHECK(s_allocator);
  DCHECK(!data_);

  int alignment = s_allocator->GetBufferAlignment();
  int padding = s_allocator->GetBufferPadding();
  allocated_size_ = size_ + padding;
  data_ = static_cast<uint8_t*>(s_allocator->Allocate(type,
                                                      allocated_size_,
                                                      alignment));
  memset(data_ + size_, 0, padding);

  if (side_data_size_ > 0)
    side_data_.reset(new uint8_t[side_data_size_]);
}
<<<<<<< HEAD
>>>>>>> af3d7334d8c ([media] Support DecoderBufferAllocator (#4348))
=======
#endif // BUILDFLAG(USE_STARBOARD_MEDIA)
>>>>>>> f071bafe36a ([media] Improve DecoderBufferAllocator logging (#5036))

// static
scoped_refptr<DecoderBuffer> DecoderBuffer::CopyFrom(
    base::span<const uint8_t> data) {
  return base::MakeRefCounted<DecoderBuffer>(base::PassKey<DecoderBuffer>(),
                                             data);
}

// static
scoped_refptr<DecoderBuffer> DecoderBuffer::FromArray(
    base::HeapArray<uint8_t> data) {
  return base::MakeRefCounted<DecoderBuffer>(base::PassKey<DecoderBuffer>(),
                                             std::move(data));
}

// static
scoped_refptr<DecoderBuffer> DecoderBuffer::FromSharedMemoryRegion(
    base::UnsafeSharedMemoryRegion region,
    uint64_t offset,
    size_t size) {
  if (size == 0) {
    return nullptr;
  }

  auto mapping = region.MapAt(offset, size);
  if (!mapping.IsValid()) {
    return nullptr;
  }

  return FromExternalMemory(
      std::make_unique<
          ExternalSharedMemoryAdapter<base::WritableSharedMemoryMapping>>(
          std::move(mapping)));
}

// static
scoped_refptr<DecoderBuffer> DecoderBuffer::FromSharedMemoryRegion(
    base::ReadOnlySharedMemoryRegion region,
    uint64_t offset,
    size_t size) {
  if (size == 0) {
    return nullptr;
  }
  auto mapping = region.MapAt(offset, size);
  if (!mapping.IsValid()) {
    return nullptr;
  }

  return FromExternalMemory(
      std::make_unique<
          ExternalSharedMemoryAdapter<base::ReadOnlySharedMemoryMapping>>(
          std::move(mapping)));
}

// static
scoped_refptr<DecoderBuffer> DecoderBuffer::FromExternalMemory(
    std::unique_ptr<ExternalMemory> external_memory) {
  DCHECK(external_memory);
  if (external_memory->Span().empty()) {
    return nullptr;
  }
  return base::MakeRefCounted<DecoderBuffer>(base::PassKey<DecoderBuffer>(),
                                             std::move(external_memory));
}

// static
scoped_refptr<DecoderBuffer> DecoderBuffer::CreateEOSBuffer(
    std::optional<ConfigVariant> next_config) {
  return base::MakeRefCounted<DecoderBuffer>(base::PassKey<DecoderBuffer>(),
                                             DecoderBufferType::kEndOfStream,
                                             std::move(next_config));
}

// static
bool DecoderBuffer::DoSubsamplesMatch(const DecoderBuffer& buffer) {
  // If buffer is at end of stream, no subsamples to verify
  if (buffer.end_of_stream()) {
    return true;
  }

  // If stream is unencrypted, we do not have to verify subsamples size.
  if (!buffer.is_encrypted()) {
    return true;
  }

  const auto& subsamples = buffer.decrypt_config()->subsamples();
  if (subsamples.empty()) {
    return true;
  }
  return VerifySubsamplesMatchSize(subsamples, buffer.size());
}

void DecoderBuffer::set_discard_padding(const DiscardPadding& discard_padding) {
  DCHECK(!end_of_stream());
  if (!side_data_ && discard_padding == DiscardPadding()) {
    return;
  }
  WritableSideData().discard_padding = discard_padding;
}

DecoderBufferSideData& DecoderBuffer::WritableSideData() {
  DCHECK(!end_of_stream());
  if (!side_data()) {
    side_data_ = std::make_unique<DecoderBufferSideData>();
  }
  return *side_data_;
}

void DecoderBuffer::set_side_data(
    std::unique_ptr<DecoderBufferSideData> side_data) {
  DCHECK(!end_of_stream());
  side_data_ = std::move(side_data);
}

bool DecoderBuffer::MatchesMetadataForTesting(
    const DecoderBuffer& buffer) const {
  if (end_of_stream() != buffer.end_of_stream()) {
    return false;
  }

  // Note: We use `side_data_` directly to avoid DCHECKs for EOS buffers.
  if (side_data_ && !side_data_->Matches(*buffer.side_data_)) {
    return false;
  }

  // None of the following methods may be called on an EOS buffer.
  if (end_of_stream()) {
    return true;
  }

  if (timestamp() != buffer.timestamp() || duration() != buffer.duration() ||
      is_key_frame() != buffer.is_key_frame()) {
    return false;
  }

  if ((decrypt_config() == nullptr) != (buffer.decrypt_config() == nullptr))
    return false;

  return decrypt_config() ? decrypt_config()->Matches(*buffer.decrypt_config())
                          : true;
}

bool DecoderBuffer::MatchesForTesting(const DecoderBuffer& buffer) const {
  if (!MatchesMetadataForTesting(buffer))  // IN-TEST
    return false;

  // It is illegal to call any member function if eos is true.
  if (end_of_stream())
    return true;

  DCHECK(!buffer.end_of_stream());
  return base::span(*this) == base::span(buffer);
}

std::string DecoderBuffer::AsHumanReadableString(bool verbose) const {
  if (end_of_stream()) {
    if (!next_config()) {
      return "EOS";
    }

    std::string config;
    const auto nc = next_config().value();
    if (const auto* ac = absl::get_if<media::AudioDecoderConfig>(&nc)) {
      config = ac->AsHumanReadableString();
    } else {
      config = absl::get<media::VideoDecoderConfig>(nc).AsHumanReadableString();
    }

    return base::StringPrintf("EOS config=(%s)", config.c_str());
  }

  std::ostringstream s;

  s << "{timestamp=" << timestamp_.InMicroseconds()
    << " duration=" << duration_.InMicroseconds() << " size=" << size()
    << " is_key_frame=" << is_key_frame_
    << " encrypted=" << (decrypt_config_ != nullptr);

  if (verbose) {
    s << " side_data=" << !!side_data();
    if (side_data()) {
      s << " discard_padding (us)=("
        << side_data_->discard_padding.first.InMicroseconds() << ", "
        << side_data_->discard_padding.second.InMicroseconds() << ")";
    }
    if (decrypt_config_) {
      s << " decrypt_config=" << (*decrypt_config_);
    }
  }

  s << "}";

  return s.str();
}

void DecoderBuffer::set_timestamp(base::TimeDelta timestamp) {
  DCHECK(!end_of_stream());
  timestamp_ = timestamp;
}

size_t DecoderBuffer::GetMemoryUsage() const {
  size_t memory_usage = sizeof(DecoderBuffer);

  if (end_of_stream()) {
    return memory_usage;
  }

  memory_usage += size();

  // Side data and decrypt config would not change after construction.
  if (side_data()) {
    memory_usage += sizeof(decltype(side_data_->spatial_layers)::value_type) *
                    side_data_->spatial_layers.capacity();
    memory_usage += side_data_->alpha_data.size();
  }
  if (decrypt_config_) {
    memory_usage += sizeof(DecryptConfig);
    memory_usage += decrypt_config_->key_id().capacity();
    memory_usage += decrypt_config_->iv().capacity();
    memory_usage +=
        sizeof(SubsampleEntry) * decrypt_config_->subsamples().capacity();
  }

  return memory_usage;
}

size_t DecoderBuffer::GetMemoryUsage() const {
  size_t memory_usage = sizeof(DecoderBuffer);

  if (end_of_stream()) {
    return memory_usage;
  }

  memory_usage += data_size();

  // Side data and decrypt config would not change after construction.
  if (side_data_size_ > 0) {
    memory_usage += side_data_size_;
  }
  if (decrypt_config_) {
    memory_usage += sizeof(DecryptConfig);
    memory_usage += decrypt_config_->key_id().capacity();
    memory_usage += decrypt_config_->iv().capacity();
    memory_usage +=
        sizeof(SubsampleEntry) * decrypt_config_->subsamples().capacity();
  }

  return memory_usage;
}

}  // namespace media

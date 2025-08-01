/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef THIRD_PARTY_BLINK_PUBLIC_PLATFORM_WEB_MEDIA_PLAYER_CLIENT_H_
#define THIRD_PARTY_BLINK_PUBLIC_PLATFORM_WEB_MEDIA_PLAYER_CLIENT_H_

<<<<<<< HEAD
=======
#include "base/time/time.h"
#include "build/build_config.h"
#include "third_party/blink/public/common/media/display_type.h"
>>>>>>> 785322efd2c ([media] [cleanup] Tidy up maxVideoCapabilities implementation (#5827))
#include "third_party/blink/public/platform/web_common.h"

namespace media {
class RemotePlaybackClientWrapper;
}  // namespace media

namespace blink {

class BLINK_PLATFORM_EXPORT WebMediaPlayerClient {
 public:
  // Returns the remote playback client associated with the media element, if
  // any.
  virtual media::RemotePlaybackClientWrapper* RemotePlaybackClientWrapper() {
    return nullptr;
  }

  // Returns the DOMNodeId of the DOM element hosting this media player.
  virtual int GetElementId() = 0;

  // Returns the DOMNodeId of the DOM element hosting this media player.
  virtual int GetElementId() = 0;

#if BUILDFLAG(USE_STARBOARD_MEDIA)
  virtual std::string GetMaxVideoCapabilities() const {return "";}
#endif //BUILDFLAG(USE_STARBOARD_MEDIA)

 protected:
  ~WebMediaPlayerClient() = default;

 private:
  friend class MediaPlayerClient;
  WebMediaPlayerClient() = default;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_PUBLIC_PLATFORM_WEB_MEDIA_PLAYER_CLIENT_H_

/******************************************************************************
 *
 *  Copyright 2022-2023 NXP
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of NXP nor the names of its contributors may be used
 * to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

#include <emvco_log.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

int osal_tml_close(int fd) {
  LOG_EMVCO_TML_D("%s\n", __func__);
  return close(fd);
}

int osal_tml_open(const char *pathname, int flags) {
  LOG_EMVCO_TML_D("%s Opening port=%s\n", __func__, pathname);
  return open(pathname, flags);
}

int osal_tml_read(int fd, void *buf, size_t count) {
  LOG_EMVCO_TML_D("%s\n", __func__);
  return read(fd, buf, count);
}

int osal_tml_write(int fd, const void *buf, size_t count) {
  LOG_EMVCO_TML_D("%s\n", __func__);
  return write(fd, buf, count);
}

int osal_tml_ioctl(int fd, unsigned long request, unsigned long reset_type,
                   int count) {
  LOG_EMVCO_TML_D("%s\n", __func__);
  if (count == 1) {
    return ioctl(fd, request);
  } else if (count == 2) {
    return ioctl(fd, request, reset_type);
  } else {
    LOG_EMVCO_TML_E(
        "%s\n  Failed. Not supported for more than one variable argument",
        __func__);
    return -1;
  }
}
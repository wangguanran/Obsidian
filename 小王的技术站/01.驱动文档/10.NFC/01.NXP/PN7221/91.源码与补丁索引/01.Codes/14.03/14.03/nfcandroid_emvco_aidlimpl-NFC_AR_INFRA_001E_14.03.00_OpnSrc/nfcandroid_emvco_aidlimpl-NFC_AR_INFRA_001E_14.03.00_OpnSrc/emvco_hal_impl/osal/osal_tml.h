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
#ifndef _PHOSALNFC_TML_H_
#define _PHOSALNFC_TML_H_
/** \addtogroup EMVCO_STACK_OSAL_API_INTERFACE
 *  @{
 */

/**
 *
 * @brief  closes the file specified by pathname
 *
 * @param[in]  pathname               path of the file.
 *
 * @return  #>0                       on success
 * @return  #-1                       error
 *
 */
int osal_tml_close(int fd);

/**
 *
 * @brief  opens the file specified by pathname
 *
 * @param[in] pathname                path of the file.
 *
 * @return  #0                         on success
 * @return  #-1                        error
 *
 */
int osal_tml_open(const char *pathname, int flags);

/**
 *
 * @brief                             attempts to read up to count bytes from
 * file descriptor fd into the buffer starting at buf
 *
 * @param[in] fd                      file descriptor of the file.
 * @param[in] buf                     buffer to store the read data
 * @param[in] count                   read up to count bytes
 *
 * @return  #0 or >0                   on success
 * @return  #-1                        error
 *
 */
int osal_tml_read(int fd, void *buf, size_t count);

/**
 *
 * @brief                          writes up to count bytes from the buffer
 * starting at buf to the file referred to by the file descriptor fd
 *
 * @param[in] fd                   file descriptor of the file.
 * @param[in] buf                  buffer to store the read data
 * @param[in] count                read up to count bytes
 *
 * @return #0 or >0                on success
 * @return #-1                     error
 *
 */
int osal_tml_write(int fd, const void *buf, size_t count);

/**
 * @brief performs a variety of control functions on STREAMS devices
 *
 * @param[in]  fd                      file descriptor of the file
 * @param[in]   request                 selects the control function
 *                                    to be performed
 * @param[in]  reset_type              NFCC power control
 * @param[in]  count                   read up to count bytes
 *
 * @return #otherthan -1              on success
 * @return #-1                        error
 *
 */
int osal_tml_ioctl(int fd, unsigned long request, unsigned long reset_type,
                   int count);
/** @}*/
#endif /* _PHOSALNFC_TML_H_*/
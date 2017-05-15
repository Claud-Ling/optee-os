/*
 * Copyright (c) 2017, Sigma Designs
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Author:  Tony He <tony_he@sigmadesigns.com>
 * Date:    2017/05/16
 *
 */

#ifndef __PTA_SVCX_H__
#define __PTA_SVCX_H__

/* This UUID is generated with uuidgen
   the ITU-T UUID generator at http://www.itu.int/ITU-T/asn1/uuid.html */
#define PTA_SVC_EXT_UUID \
		{ 0x4AB65334, 0x5A59, 0x45D2,  \
			{ 0xA9, 0x61, 0x1F, 0x2F, 0x0B, 0xED, 0x2B, 0xD6 } }

/* The TAFs ID implemented in this TA */
#define PTA_SVCX_CMD_HELLO		0
#define PTA_SVCX_CMD_SELF_TESTS		1
#define PTA_SVCX_CMD_MMAP		2
#define PTA_SVCX_CMD_MUNMAP		3
#define PTA_SVCX_CMD_OTP_WRITE		4

#ifndef __ASSEMBLY__

/*
 * user map attributes constants
 */
enum utee_umap_attr {
	TEE_MAP_ATTR_NONCACHE = 0,
	TEE_MAP_ATTR_CACHED = (1 << 0),
	TEE_MAP_ATTR_GLOBAL = (1 << 3),
	TEE_MAP_ATTR_SECURE = (1 << 4),
};


#define XMAP_ATTR_MASK		0xF
#define XMAP_NNORMAL		TEE_MAP_ATTR_CACHED	/*normal memory (cached)*/
#define XMAP_NDEVICE		TEE_MAP_ATTR_NONCACHE	/*none-secure device memory*/
#define XMAP_SNORMAL		(TEE_MAP_ATTR_CACHED |	\
				 TEE_MAP_ATTR_SECURE)	/*secure normal memory (cached)*/
#define XMAP_SDEVICE		(TEE_MAP_ATTR_NONCACHE |\
				 TEE_MAP_ATTR_SECURE)	/*secure device memory*/

/*
 * user map protection constants
 */
enum utee_umap_prot {
	TEE_MAP_PROT_READ = (1 << 0),
	TEE_MAP_PROT_WRITE = (1 << 1),
	TEE_MAP_PROT_EXEC = (1 << 2),
};

#endif /* !__ASSEMBLY__ */
#endif /* __PTA_SVC_EXT_H__ */

/*
 * Copyright (c) 2016, Marie Helene Kvello-Aune
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * thislist of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD$
 */


#include <net/if.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>


#include "libifc.h" // Needed for libifc_errstate
#include "libifc_internal.h"

int
libifc_ioctlwrap_ret(libifc_handle_t *h, unsigned long request, int rcode)
{
	if (rcode != 0) {
		h->error.errtype = IOCTL;
		h->error.ioctl_request = request;
		h->error.errcode = errno;
	}
	return (rcode);
}


int
libifc_ioctlwrap(libifc_handle_t *h, const int addressfamily,
    unsigned long request, struct ifreq *ifr)
{
	int s;

	if (libifc_socket(h, addressfamily, &s) != 0) {
		return (-1);
	}

	int rcode = ioctl(s, request, ifr);
	return (libifc_ioctlwrap_ret(h, request, rcode));
}


/*
 * Function to get socket for the specified address family.
 * If the socket doesn't already exist, attempt to create it.
 */
int libifc_socket(libifc_handle_t *h, const int addressfamily, int *s)
{
	if (addressfamily > AF_MAX) {
		h->error.errtype = SOCKET;
		h->error.errcode = EINVAL;
		return (-1);
	}

	if (h->sockets[addressfamily] != -1) {
		*s = h->sockets[addressfamily];
		return (0);
	}

	/* We don't have a socket of that type available. Create one. */
	h->sockets[addressfamily] = socket(addressfamily, SOCK_DGRAM, 0);
	if (h->sockets[addressfamily] == -1) {
		h->error.errtype = SOCKET;
		h->error.errcode = errno;
		return (-1);
	}

	*s = h->sockets[addressfamily];
	return (0);
}

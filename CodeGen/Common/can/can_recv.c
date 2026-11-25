/*
COPYRIGHT (C) 2024 Michal Lenc (michallenc@seznam.cz)
COPYRIGHT (C) 2026 Ahmad Hisham (ahmadhisham25656@gmail.com)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
*/

#include <pyblock.h>

#include <candev.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <candev.h>
#include "candata.h"


static void init(python_block *block) {
  block->ptrPar = NULL;

  if (pysim_can_initialize() != 0) {
    return;
  }

  struct can_block_ctx *ctx = malloc(sizeof(*ctx));
  if (ctx == NULL) {
    return;
  }

  char* dev_node = block -> str;
  ctx->fd = pysim_can_open(dev_node);
  if (ctx->fd < 0) {
    printf("%s open failed %d\n", dev_node, ctx->fd);
    free(ctx);
    return;
  }

  block->ptrPar = ctx;
  uint32_t ide = block->intPar[0];
  pysim_can_register_msg(ctx->fd, ide);
}

static void inout(python_block *block) {
  struct can_block_ctx *ctx = block->ptrPar;
  if (ctx == NULL) {
    return;
  }

  int8_t payload[64] = {0};
  uint16_t len;
  uint32_t id = (uint32_t)block->intPar[0];

  const int *bits = &block->intPar[1];
  const int *types = &block->intPar[1 + block->nout];
  unsigned offset = 0;
  const unsigned capacity = sizeof payload * 8u;

  int result = pysim_can_read_msg(ctx->fd, id, &len, payload);
  if (result != 0) {
    fprintf(stderr, "CAN receive: Failed read.");
    return;
  }
  if (len > sizeof(payload) || len <= 0) {
    fprintf(stderr, "CAN receive: unexpected length of CAN frame data is %d\n", len);
    return;
  }

  for (int i = 0; i < block->nout; ++i) {
    if (bits[i] <= 0) {
      fprintf(stderr, "CAN receive: invalid width on output %d\n", i);
      return;
    }

    unsigned width = (unsigned)bits[i];

    if (width > capacity - offset) {
      fprintf(stderr, "CAN receive: payload exceeds %zu bytes\n", sizeof(payload));
      return;
    }

    double *y = block->y[i];

    if (y == NULL || !can_get_as_double(payload, len, offset, width, y, (canopen_data_type_t)types[i])) {
      fprintf(stderr, "CAN receive: cannot encode output %d\n", i);
      return;
    }

    offset += width;
  }

  // double value;
  // memcpy(&value, payload, sizeof(value));
}

static void end(python_block *block) {
  struct can_block_ctx *ctx = block->ptrPar;

  if (ctx == NULL) return;

  free(ctx);
  block->ptrPar = NULL;
}

void can_recv(int flag, python_block *block)
{
  if (flag==CG_OUT) {          /* get input */
    inout(block);
  }
  else if (flag==CG_END) {     /* termination */
    end(block);
  }
  else if (flag ==CG_INIT) {    /* initialisation */
    init(block);
  }
}

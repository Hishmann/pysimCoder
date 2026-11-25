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
}

static void inout(python_block *block) {
  struct can_block_ctx *ctx = block->ptrPar;
  if (ctx == NULL) {
    return;
  }

  uint32_t ide = block->intPar[0];

  int8_t payload[8] = {0};

  uint32_t id = (uint32_t)block->intPar[0];
  const int *bits = &block->intPar[1];
  const int *types = &block->intPar[1 + block->nin];
  unsigned offset = 0;
  const unsigned capacity = sizeof payload * 8u;

  for (int i = 0; i < block->nin; ++i) {
    if (bits[i] <= 0) {
      fprintf(stderr, "CAN send: invalid width on input %d\n", i);
      return;
    }

    unsigned width = (unsigned)bits[i];

    if (width > capacity - offset) {
      fprintf(stderr, "CAN send: payload exceeds %d bytes\n", sizeof(payload));
      return;
    }

    const double *u = block->u[i];

    if (u == NULL || !can_put_from_double(payload, sizeof payload, offset, width, u[0], (canopen_data_type_t)types[i])) {
      fprintf(stderr, "CAN send: cannot encode input %d\n", i);
      return;
    }

    offset += width;
  }

  int result = pysim_can_send_msg(ctx->fd, ide, sizeof(payload), (uint8_t *)payload);
  if (result != 0) fprintf(stderr, "CAN send failed: %d\n", result);

  // pysim_can_send_msg(ctx->fd, ide, len, data);
}

static void end(python_block *block) {
  struct can_block_ctx *ctx = block->ptrPar;

  if (ctx == NULL) return;

  free(ctx);
  block->ptrPar = NULL;
}

void can_send(int flag, python_block *block)
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

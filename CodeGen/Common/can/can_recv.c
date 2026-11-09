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

  uint8_t data[64];
  uint16_t len;
  uint32_t ide = block->intPar[0];

  int result = pysim_can_read_msg(ctx->fd, ide, &len, data);
  if (result != 0) return;
  if (len != sizeof(double)) return;  // Unexpected payload size

  double value;
  memcpy(&value, data, sizeof(value));

  double *y = block->y[0];
  y[0] = value;
}

static void end(python_block *block) {
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

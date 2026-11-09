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
}

static void inout(python_block *block) {
  struct can_block_ctx *ctx = block->ptrPar;
  if (ctx == NULL) {
    return;
  }

  uint32_t ide = block->intPar[0];
  double *u = block->u[0];

  uint8_t *data = (uint8_t *)u;
  uint16_t len = sizeof(double);

  pysim_can_send_msg(ctx->fd, ide, len, data);
}

static void end(python_block *block) {
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

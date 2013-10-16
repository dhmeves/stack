/*
 * EFCP related utilities
 *
 *    Francesco Salvestrini <f.salvestrini@nextworks.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef EFCP_UTILS_H
#define EFCP_UTILS_H

#include "common.h"
#include "efcp.h"

struct efcp_imap;

struct efcp_imap * efcp_imap_create(void);
int                efcp_imap_destroy(struct efcp_imap * map,
                                     int (* destructor)(struct efcp * i));

int                efcp_imap_empty(struct efcp_imap * map);
int                efcp_imap_add(struct efcp_imap * map,
                                 cep_id_t           key,
                                 struct efcp *      value);
struct efcp *      efcp_imap_find(struct efcp_imap * map,
                                  cep_id_t           key);
int                efcp_imap_update(struct efcp_imap * map,
                                    cep_id_t           key,
                                    struct efcp *      value);
int                efcp_imap_remove(struct efcp_imap * map,
                                    cep_id_t           key);

struct cidm;

/* ALWAYS use this function to get a bad port-id */
cep_id_t            cep_id_bad(void);

/* ALWAYS use this function to check if the id looks good */
int                 is_cep_id_ok(cep_id_t id);

struct cidm *       cidm_create(void);
int                 cidm_destroy(struct cidm * instance);

cep_id_t            cidm_allocate(struct cidm * instance);
int                 cidm_release(struct cidm * instance,
                                 cep_id_t      cep_id);

#endif

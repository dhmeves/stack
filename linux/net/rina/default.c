/*
 * RINA default personality
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

#include <linux/module.h>

#define RINA_PREFIX "personality-default"

#include "logs.h"
#include "utils.h"
#include "personality.h"
#include "netlink.h"
#include "kipcm.h"
#include "efcp.h"
#include "rmt.h"
#include "debug.h"

#define DEFAULT_LABEL "default"

struct personality_data {
        struct kipcm *       kipcm;
        struct rina_nl_set * nlset;

        /* FIXME: Types to be rearranged */
        void *               efcp;
        void *               rmt;
};

static int is_personality_ok(const struct personality_data * p)
{
        if (!p)
                return 0;
        if (!p->kipcm)
                return 0;
        if (!p->efcp)
                return 0;

        return 1;
} 

static int default_ipc_create(struct personality_data * data,
                              const struct name *       name,
                              ipc_process_id_t          id,
                              const char *              type)
{
        if (!is_personality_ok(data)) return -1;

        LOG_DBG("Calling wrapped function");

        return kipcm_ipcp_create(data->kipcm, name, id, type);
}

static int default_ipc_configure(struct personality_data *  data,
                                 ipc_process_id_t           id,
                                 const struct ipcp_config * configuration)
{
        if (!is_personality_ok(data)) return -1;

        LOG_DBG("Calling wrapped function");

        return kipcm_ipcp_configure(data->kipcm, id, configuration);
}

static int default_ipc_destroy(struct personality_data * data,
                               ipc_process_id_t          id)
{
        if (!is_personality_ok(data)) return -1;

        LOG_DBG("Calling wrapped function");

        return kipcm_ipcp_destroy(data->kipcm, id);
}

static int default_connection_create(struct personality_data * data,
                                     const struct connection * connection)
{
        cep_id_t id; /* FIXME: Remains unused !!! */

        if (!is_personality_ok(data)) return -1;

        LOG_DBG("Calling wrapped function");

        return efcp_create(data->efcp, connection, &id);
}

static int default_connection_destroy(struct personality_data * data,
                                      cep_id_t                  id)
{
        if (!is_personality_ok(data)) return -1;

        LOG_DBG("Calling wrapped function");

        return efcp_destroy(data->efcp, id);
}

static int default_connection_update(struct personality_data * data,
                                     cep_id_t                  id_from,
                                     cep_id_t                  id_to)
{
        if (!is_personality_ok(data)) return -1;

        LOG_DBG("Calling wrapped function");

        return efcp_update(data->efcp, id_from, id_to);
}

static int default_sdu_write(struct personality_data * data,
                             port_id_t                 id,
                             struct sdu *              sdu)
{
        if (!is_personality_ok(data)) return -1;

        LOG_DBG("Calling wrapped function");

        return kipcm_sdu_write(data->kipcm, id, sdu);
}

static int default_sdu_read(struct personality_data * data,
                            port_id_t                 id,
                            struct sdu **             sdu)
{
        if (!is_personality_ok(data)) return -1;

        LOG_DBG("Calling wrapped function");

        return kipcm_sdu_read(data->kipcm, id, sdu);
}

static int default_fini(struct personality_data * data)
{
        struct personality_data * tmp = data;
        int                       err;

        if (!data) {
                LOG_ERR("Personality data is bogus, cannot finalize "
                        "default personality");
                return -1;
        }

        LOG_DBG("Finalizing default personality");

        if (tmp->rmt) {
                err = rmt_fini(tmp->rmt);
                if (err) return err;
        }
        if (tmp->efcp) {
                err = efcp_fini(tmp->efcp);
                if (err) return err;
        }
        if (tmp->kipcm) {
                err = kipcm_fini(tmp->kipcm, tmp->nlset);
                if (err) return err;
        }
        if (tmp->nlset) {
                err = rina_netlink_set_destroy(tmp->nlset);
                if (err) return err;
        }

        LOG_DBG("Default personality finalized successfully");

        return 0;
}

/* FIXME: To be removed ABSOLUTELY */
struct kipcm * default_kipcm = NULL;
EXPORT_SYMBOL(default_kipcm);

static int default_init(struct kobject *          parent,
                        personality_id            id,
                        struct personality_data * data)
{
        if (!data) {
                LOG_ERR("Personality data is bogus, cannot initialize "
                        "default personality");
                return -1;
        }

        LOG_DBG("Initializing default personality");

        LOG_DBG("Initializing default Netlink component");
        data->nlset = rina_netlink_set_create(id);
        if (!data->nlset) {
                if (default_fini(data)) {
                        LOG_CRIT("The system might become unstable ...");
                        return -1;
                }
        }

        LOG_DBG("Initializing kipcm component");
        data->kipcm = kipcm_init(parent, data->nlset);
        if (!data->kipcm) {
                if (default_fini(data)) {
                        LOG_CRIT("The system might become unstable ...");
                        return -1;
                }
        }

        /* FIXME: To be removed */
        default_kipcm = data->kipcm;

        LOG_DBG("Initializing efcp component");
        data->efcp = efcp_init(parent);
        if (!data->efcp) {
                if (default_fini(data)) {
                        LOG_CRIT("The system might become unstable ...");
                        return -1;
                }
        }

        LOG_DBG("Initializing rmt component");
        data->rmt = rmt_init(parent);
        if (!data->rmt) {
                if (default_fini(data)) {
                        LOG_CRIT("The system might become unstable ...");
                        return -1;
                }
        }

        LOG_DBG("Default personality initialized successfully");

        return 0;
}

struct personality_ops ops = {
        .init               = default_init,
        .fini               = default_fini,
        .ipc_create         = default_ipc_create,
        .ipc_configure      = default_ipc_configure,
        .ipc_destroy        = default_ipc_destroy,
        .sdu_read           = default_sdu_read,
        .sdu_write          = default_sdu_write,
        .connection_create  = default_connection_create,
        .connection_destroy = default_connection_destroy,
        .connection_update  = default_connection_update,
};

static struct personality_data data;

static struct personality *    personality = NULL;

static int __init mod_init(void)
{
        LOG_DBG("Rina default personality loading");

        if (personality) {
                LOG_ERR("Rina default personality already initialized, "
                        "bailing out");
                return -1;
        }

        LOG_DBG("Finally registering personality");
        personality = rina_personality_register("default", &data, &ops);
        if (!personality)
                return -1;

        ASSERT(personality != NULL);

        LOG_DBG("Rina default personality loaded successfully");

        return 0;
}

static void __exit mod_exit(void)
{
        LOG_DBG("Rina default personality unloading");

        ASSERT(personality != NULL);

        if (rina_personality_unregister(personality)) {
                LOG_CRIT("Got problems while unregistering personality, "
                         "bailing out");
                return;
        }

        personality = NULL;

        LOG_DBG("Rina default personality unloaded successfully");
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_DESCRIPTION("RINA default personality");

MODULE_LICENSE("GPL");

MODULE_AUTHOR("Francesco Salvestrini <f.salvestrini@nextworks.it>");

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */

/*
 * Dan Williams <dcbw@redhat.com>
 * Tambet Ingo <tambet@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 * (C) Copyright 2007 - 2008 Red Hat, Inc.
 * (C) Copyright 2007 - 2008 Novell, Inc.
 */

#ifndef NM_CONNECTION_H
#define NM_CONNECTION_H

#include <glib.h>
#include <glib-object.h>
#include <nm-setting.h>

G_BEGIN_DECLS

#define NM_TYPE_CONNECTION            (nm_connection_get_type ())
#define NM_CONNECTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), NM_TYPE_CONNECTION, NMConnection))
#define NM_CONNECTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), NM_TYPE_CONNECTION, NMConnectionClass))
#define NM_IS_CONNECTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NM_TYPE_CONNECTION))
#define NM_IS_CONNECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), NM_TYPE_CONNECTION))
#define NM_CONNECTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), NM_TYPE_CONNECTION, NMConnectionClass))

typedef enum {
	NM_CONNECTION_SCOPE_UNKNOWN = 0,
	NM_CONNECTION_SCOPE_SYSTEM,
	NM_CONNECTION_SCOPE_USER
} NMConnectionScope;

#define NM_CONNECTION_SCOPE "scope"
#define NM_CONNECTION_PATH "path"

typedef struct {
	GObject parent;
} NMConnection;

typedef struct {
	GObjectClass parent;

	/* Signals */
	void (*secrets_updated) (NMConnection *connection, const char * setting);
} NMConnectionClass;

GType nm_connection_get_type (void);

NMConnection *nm_connection_new           (void);

NMConnection *nm_connection_new_from_hash (GHashTable *hash, GError **error);

NMConnection *nm_connection_duplicate     (NMConnection *connection);

void          nm_connection_add_setting   (NMConnection *connection,
								   NMSetting    *setting);

void          nm_connection_remove_setting (NMConnection *connection,
								    GType         setting_type);

NMSetting    *nm_connection_get_setting   (NMConnection *connection,
								   GType         setting_type);

NMSetting    *nm_connection_get_setting_by_name (NMConnection *connection,
									    const char *name);

gboolean      nm_connection_replace_settings (NMConnection *connection,
									 GHashTable *new_settings);

/* Returns TRUE if the connections are the same */
gboolean      nm_connection_compare       (NMConnection *connection,
                                           NMConnection *other,
                                           NMSettingCompareFlags flags);

gboolean      nm_connection_verify        (NMConnection *connection, GError **error);

const char *  nm_connection_need_secrets  (NMConnection *connection,
                                           GPtrArray **hints);

void          nm_connection_clear_secrets (NMConnection *connection);

void          nm_connection_update_secrets (NMConnection *connection,
                                            const char *setting_name,
                                            GHashTable *secrets);

void             nm_connection_set_scope (NMConnection *connection,
                                                 NMConnectionScope scope);

NMConnectionScope nm_connection_get_scope (NMConnection *connection);

void             nm_connection_set_path (NMConnection *connection,
                                         const char *path);

const char *     nm_connection_get_path (NMConnection *connection);

void          nm_connection_for_each_setting_value (NMConnection *connection,
										  NMSettingValueIterFn func,
										  gpointer user_data);

GHashTable   *nm_connection_to_hash       (NMConnection *connection);
void          nm_connection_dump          (NMConnection *connection);

NMSetting    *nm_connection_create_setting (const char *name);

void nm_setting_register   (const char *name,
					   GType type);

void nm_setting_unregister (const char *name);

GType nm_connection_lookup_setting_type (const char *name);

GType nm_connection_lookup_setting_type_by_quark (GQuark error_quark);

G_END_DECLS

#endif /* NM_CONNECTION_H */

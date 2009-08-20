/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2009 Anders F Bjorklund <afb@users.sourceforge.net>
 *
 * Licensed under the GNU General Public License Version 2
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <pk-backend.h>
#include <pk-backend-spawn.h>

static PkBackendSpawn *spawn = 0;
static const gchar* BACKEND_FILE = "portsBackend.rb";

/**
 * backend_initialize:
 * This should only be run once per backend load, i.e. not every transaction
 */
static void
backend_initialize (PkBackend *backend)
{
	egg_debug ("backend: initialize");
	spawn = pk_backend_spawn_new ();
	pk_backend_spawn_set_name (spawn, "ports");
	/* allowing sigkill as long as no one complain */
	pk_backend_spawn_set_allow_sigkill (spawn, TRUE);
}

/**
 * backend_destroy:
 * This should only be run once per backend load, i.e. not every transaction
 */
static void
backend_destroy (PkBackend *backend)
{
	egg_debug ("backend: destroy");
	g_object_unref (spawn);
}

/**
 * backend_get_groups:
 */
static PkBitfield
backend_get_groups (PkBackend *backend)
{
	return pk_bitfield_from_enums (
			PK_GROUP_ENUM_ACCESSIBILITY,
			PK_GROUP_ENUM_ACCESSORIES,
			PK_GROUP_ENUM_ADMIN_TOOLS,
			PK_GROUP_ENUM_COMMUNICATION,
			PK_GROUP_ENUM_DESKTOP_GNOME,
			PK_GROUP_ENUM_DESKTOP_KDE,
			PK_GROUP_ENUM_DESKTOP_OTHER,
			PK_GROUP_ENUM_DESKTOP_XFCE,
		/*	PK_GROUP_ENUM_EDUCATION, */
			PK_GROUP_ENUM_FONTS,
			PK_GROUP_ENUM_GAMES,
			PK_GROUP_ENUM_GRAPHICS,
			PK_GROUP_ENUM_INTERNET,
		/*	PK_GROUP_ENUM_LEGACY, */
			PK_GROUP_ENUM_LOCALIZATION,
		/*	PK_GROUP_ENUM_MAPS, */
			PK_GROUP_ENUM_MULTIMEDIA,
			PK_GROUP_ENUM_NETWORK,
		/*	PK_GROUP_ENUM_OFFICE, */
			PK_GROUP_ENUM_OTHER,
		/*	PK_GROUP_ENUM_POWER_MANAGEMENT, */
			PK_GROUP_ENUM_PROGRAMMING,
			PK_GROUP_ENUM_PUBLISHING,
		/*	PK_GROUP_ENUM_REPOS, */
			PK_GROUP_ENUM_SECURITY,
			PK_GROUP_ENUM_SERVERS,
			PK_GROUP_ENUM_SYSTEM,
			PK_GROUP_ENUM_VIRTUALIZATION,
			PK_GROUP_ENUM_SCIENCE,
			PK_GROUP_ENUM_DOCUMENTATION,
		/*	PK_GROUP_ENUM_ELECTRONICS, */
		/*	PK_GROUP_ENUM_COLLECTIONS, */
		/*	PK_GROUP_ENUM_VENDOR, */
		/*	PK_GROUP_ENUM_NEWEST, */
			-1);
}

/**
 * backend_get_filters:
 */
static PkBitfield
backend_get_filters (PkBackend *backend)
{
	return pk_bitfield_from_enums (
			PK_FILTER_ENUM_INSTALLED,
			-1);
}

/**
 * backend_cancel:
 */
static void
backend_cancel (PkBackend *backend)
{
	/* this feels bad... */
	pk_backend_spawn_kill (spawn);
}

/**
 * backend_get_depends:
 */
static void
backend_get_depends (PkBackend *backend, PkBitfield filters, gchar **package_ids, gboolean recursive)
{
	gchar *filters_text;
	gchar *package_ids_temp;

	package_ids_temp = pk_package_ids_to_text (package_ids);
	filters_text = pk_filter_bitfield_to_text (filters);
	pk_backend_spawn_helper (spawn, BACKEND_FILE, "get-depends", filters_text, package_ids_temp, pk_backend_bool_to_text (recursive), NULL);
	g_free (package_ids_temp);
	g_free (filters_text);
}

/**
 * backend_get_details:
 */
static void
backend_get_details (PkBackend *backend, gchar **package_ids)
{
	gchar *package_ids_temp;

	package_ids_temp = pk_package_ids_to_text (package_ids);
	pk_backend_spawn_helper (spawn, BACKEND_FILE, "get-details", package_ids_temp, NULL);
	g_free (package_ids_temp);
}

/**
 * backend_get_files:
 */
static void
backend_get_files (PkBackend *backend, gchar **package_ids)
{
	gchar *package_ids_temp;

	package_ids_temp = pk_package_ids_to_text (package_ids);
	pk_backend_spawn_helper (spawn, BACKEND_FILE, "get-files", package_ids_temp, NULL);
	g_free (package_ids_temp);
}

/**
 * backend_refresh_cache:
 */
static void
backend_refresh_cache (PkBackend *backend, gboolean force)
{
	/* check network state */
	if (!pk_backend_is_online (backend)) {
		pk_backend_error_code (backend, PK_ERROR_ENUM_NO_NETWORK, "Cannot refresh cache whilst offline");
		pk_backend_finished (backend);
		return;
	}

	pk_backend_spawn_helper (spawn, BACKEND_FILE, "refresh-cache", pk_backend_bool_to_text (force), NULL);
}

/**
 * pk_backend_resolve:
 */
static void
backend_resolve (PkBackend *backend, PkBitfield filters, gchar **package_ids)
{
	gchar *filters_text;
	gchar *package_ids_temp;

	filters_text = pk_filter_bitfield_to_text (filters);
	package_ids_temp = pk_package_ids_to_text (package_ids);
	pk_backend_spawn_helper (spawn, BACKEND_FILE, "resolve", filters_text, package_ids_temp, NULL);
	g_free (package_ids_temp);
	g_free (filters_text);
}

/**
 * pk_backend_search_details:
 */
static void
backend_search_details (PkBackend *backend, PkBitfield filters, const gchar *search)
{
	gchar *filters_text;

	filters_text = pk_filter_bitfield_to_text (filters);
	pk_backend_spawn_helper (spawn, BACKEND_FILE, "search-details", filters_text, search, NULL);
	g_free (filters_text);
}

/**
 * backend_search_file:
 */
static void
backend_search_file (PkBackend *backend, PkBitfield filters, const gchar *search)
{
	gchar *filters_text;

	filters_text = pk_filter_bitfield_to_text (filters);
	pk_backend_spawn_helper (spawn, BACKEND_FILE, "search-file", filters_text, search, NULL);
	g_free (filters_text);
}

/**
 * pk_backend_search_group:
 */
static void
backend_search_group (PkBackend *backend, PkBitfield filters, const gchar *search)
{
	gchar *filters_text;

	filters_text = pk_filter_bitfield_to_text (filters);
	pk_backend_spawn_helper (spawn, BACKEND_FILE, "search-group", filters_text, search, NULL);
	g_free (filters_text);
}

/**
 * backend_search_name:
 */
static void
backend_search_name (PkBackend *backend, PkBitfield filters, const gchar *search)
{
	gchar *filters_text;

	filters_text = pk_filter_bitfield_to_text (filters);
	pk_backend_spawn_helper (spawn, BACKEND_FILE, "search-name", filters_text, search, NULL);
	g_free (filters_text);
}

/**
 * backend_get_packages:
 */
static void
backend_get_packages (PkBackend *backend, PkBitfield filters)
{
	gchar *filters_text;

	filters_text = pk_filter_bitfield_to_text (filters);
	pk_backend_spawn_helper (spawn, BACKEND_FILE, "get-packages", filters_text, NULL);
	g_free (filters_text);
}

/**
 * pk_backend_get_repo_list:
 */
static void
backend_get_repo_list (PkBackend *backend, PkBitfield filters)
{
	gchar *filters_text;

	filters_text = pk_filter_bitfield_to_text (filters);
	pk_backend_spawn_helper (spawn, BACKEND_FILE, "get-repo-list", filters_text, NULL);
	g_free (filters_text);
}

/**
 * backend_get_requires:
 */
static void
backend_get_requires (PkBackend *backend, PkBitfield filters, gchar **package_ids, gboolean recursive)
{
	gchar *package_ids_temp;
	gchar *filters_text;

	package_ids_temp = pk_package_ids_to_text (package_ids);
	filters_text = pk_filter_bitfield_to_text (filters);
	pk_backend_spawn_helper (spawn, BACKEND_FILE, "get-requires", filters_text, package_ids_temp, pk_backend_bool_to_text (recursive), NULL);
	g_free (filters_text);
	g_free (package_ids_temp);
}

PK_BACKEND_OPTIONS (
	"Ports",				/* description */
	"Anders F Björklund <afb@users.sourceforge.net>",	/* author */
	backend_initialize,			/* initalize */
	backend_destroy,			/* destroy */
	backend_get_groups,			/* get_groups */
	backend_get_filters,			/* get_filters */
	NULL,			/* get_mime_types */
	backend_cancel,				/* cancel */
	NULL,		/* download_packages */
	NULL,					/* get_categories */
	backend_get_depends,			/* get_depends */
	backend_get_details,			/* get_details */
	NULL,		/* get_distro_upgrades */
	backend_get_files,			/* get_files */
	backend_get_packages,			/* get_packages */
	backend_get_repo_list,			/* get_repo_list */
	backend_get_requires,			/* get_requires */
	NULL,		/* get_update_detail */
	NULL,			/* get_updates */
	NULL,			/* install_files */
	NULL,		/* install_packages */
	NULL,			/* install_signature */
	backend_refresh_cache,			/* refresh_cache */
	NULL,		/* remove_packages */
	NULL,			/* repo_enable */
	NULL,			/* repo_set_data */
	backend_resolve,			/* resolve */
	NULL,			/* rollback */
	backend_search_details,			/* search_details */
	backend_search_file,			/* search_file */
	backend_search_group,			/* search_group */
	backend_search_name,			/* search_name */
	NULL,		/* update_packages */
	NULL,			/* update_system */
	NULL			/* what_provides */
);

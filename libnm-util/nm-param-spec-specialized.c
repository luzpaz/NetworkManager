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

#include "nm-param-spec-specialized.h"

struct _NMParamSpecSpecialized {
	GParamSpec parent;
};

#include <string.h>
#include <math.h>
#include <dbus/dbus-glib.h>

/***********************************************************/
/* nm_gvalues_compare */

static gint nm_gvalues_compare (const GValue *value1, const GValue *value2);

static gboolean
type_is_fixed_size (GType type)
{
	switch (type) {
	case G_TYPE_CHAR:
	case G_TYPE_UCHAR:
	case G_TYPE_BOOLEAN:
	case G_TYPE_LONG:
	case G_TYPE_ULONG:
	case G_TYPE_INT:
	case G_TYPE_UINT:
	case G_TYPE_INT64:
	case G_TYPE_UINT64:
	case G_TYPE_FLOAT:
	case G_TYPE_DOUBLE:
		return TRUE;
	default:
		return FALSE;
	}
}

#define FLOAT_FACTOR 0.00000001

static gint
nm_gvalues_compare_fixed (const GValue *value1, const GValue *value2)
{
	int ret = 0;

	switch (G_VALUE_TYPE (value1)) {
	case G_TYPE_CHAR: {
		gchar val1 = g_value_get_char (value1);
		gchar val2 = g_value_get_char (value2);
		if (val1 != val2)
			ret = val1 < val2 ? -1 : val1 > val2;
		break;
	}
	case G_TYPE_UCHAR: {
		guchar val1 = g_value_get_uchar (value1);
		guchar val2 = g_value_get_uchar (value2);
		if (val1 != val2)
			ret = val1 < val2 ? -1 : val1 > val2;
		break;
	}
	case G_TYPE_BOOLEAN: {
		gboolean val1 = g_value_get_boolean (value1);
		gboolean val2 = g_value_get_boolean (value2);
		if (val1 != val2)
			ret = val1 < val2 ? -1 : val1 > val2;
		break;
	}
	case G_TYPE_LONG: {
		glong val1 = g_value_get_long (value1);
		glong val2 = g_value_get_long (value2);
		if (val1 != val2)
			ret = val1 < val2 ? -1 : val1 > val2;
		break;
	}
	case G_TYPE_ULONG: {
		gulong val1 = g_value_get_ulong (value1);
		gulong val2 = g_value_get_ulong (value2);
		if (val1 != val2)
			ret = val1 < val2 ? -1 : val1 > val2;
		break;
	}
	case G_TYPE_INT: {
		gint val1 = g_value_get_int (value1);
		gint val2 = g_value_get_int (value2);
		if (val1 != val2)
			ret = val1 < val2 ? -1 : val1 > val2;
		break;
	}
	case G_TYPE_UINT: {
		guint val1 = g_value_get_uint (value1);
		guint val2 = g_value_get_uint (value2);
		if (val1 != val2)
			ret = val1 < val2 ? -1 : val1 > val2;
		break;
	}
	case G_TYPE_INT64: {
		gint64 val1 = g_value_get_int64 (value1);
		gint64 val2 = g_value_get_int64 (value2);
		if (val1 != val2)
			ret = val1 < val2 ? -1 : val1 > val2;
		break;
	}
	case G_TYPE_UINT64: {
		guint64 val1 = g_value_get_uint64 (value1);
		guint64 val2 = g_value_get_uint64 (value2);
		if (val1 != val2)
			ret = val1 < val2 ? -1 : val1 > val2;
		break;
	}
	case G_TYPE_FLOAT: {
		gfloat val1 = g_value_get_float (value1);
		gfloat val2 = g_value_get_float (value2);
		/* Can't use == or != here due to inexactness of FP */
		if (fabsf (val1 - val2) > FLOAT_FACTOR)
			ret = val1 < val2 ? -1 : val1 > val2;
		break;
	}
	case G_TYPE_DOUBLE: {
		gdouble val1 = g_value_get_double (value1);
		gdouble val2 = g_value_get_double (value2);
		if (fabs (val1 - val2) > FLOAT_FACTOR)
			ret = val1 < val2 ? -1 : val1 > val2;
		break;
	}
	default:
		g_warning ("Unhandled fixed size type '%s'", G_VALUE_TYPE_NAME (value1));
	}

	return ret;
}

static gint
nm_gvalues_compare_string (const GValue *value1, const GValue *value2)
{
	const char *str1 = g_value_get_string (value1);
	const char *str2 = g_value_get_string (value2);

	if (str1 == str2)
		return 0;

	if (!str1)
		return 1;
	if (!str2)
		return -1;

	return strcmp (str1, str2);
}

static gint
nm_gvalues_compare_strv (const GValue *value1, const GValue *value2)
{
	char **strv1;
	char **strv2;
	gint ret;
	guint i = 0;

	strv1 = (char **) g_value_get_boxed (value1);
	strv2 = (char **) g_value_get_boxed (value2);

	while (strv1[i] && strv2[i]) {
		ret = strcmp (strv1[i], strv2[i]);
		if (ret)
			return ret;
		i++;
	}

	if (strv1[i] == NULL && strv2[i] == NULL)
		return 0;

	if (strv1[i])
		return 1;

	return -1;
}

static void
nm_gvalue_destroy (gpointer data)
{
	GValue *value = (GValue *) data;

	g_value_unset (value);
	g_slice_free (GValue, value);
}

static GValue *
nm_gvalue_dup (const GValue *value)
{
	GValue *dup;

	dup = g_slice_new0 (GValue);
	g_value_init (dup, G_VALUE_TYPE (value));
	g_value_copy (value, dup);

	return dup;
}

static void
iterate_collection (const GValue *value, gpointer user_data)
{
	GSList **list = (GSList **) user_data;
	
	*list = g_slist_prepend (*list, nm_gvalue_dup (value));
}

static gint
nm_gvalues_compare_collection (const GValue *value1, const GValue *value2)
{
	gint ret;
	guint len1;
	guint len2;
	GType value_type = dbus_g_type_get_collection_specialization (G_VALUE_TYPE (value1));

	if (type_is_fixed_size (value_type)) {
		gpointer data1 = NULL;
		gpointer data2 = NULL;

		dbus_g_type_collection_get_fixed ((GValue *) value1, &data1, &len1);
		dbus_g_type_collection_get_fixed ((GValue *) value2, &data2, &len2);

		if (len1 != len2)
			ret = len1 < len2 ? -1 : len1 > len2;
		else
			ret = memcmp (data1, data2, len1);
	} else {
		GSList *list1 = NULL;
		GSList *list2 = NULL;

		dbus_g_type_collection_value_iterate (value1, iterate_collection, &list1);
		len1 = g_slist_length (list1);
		dbus_g_type_collection_value_iterate (value2, iterate_collection, &list2);
		len2 = g_slist_length (list2);

		if (len1 != len2)
			ret = len1 < len2 ? -1 : len1 > len2;
		else {
			GSList *iter1;
			GSList *iter2;

			for (iter1 = list1, iter2 = list2, ret = 0;
				ret == 0 && iter1 && iter2; 
				iter1 = iter1->next, iter2 = iter2->next)
				ret = nm_gvalues_compare ((GValue *) iter1->data, (GValue *) iter2->data);
		}

		g_slist_foreach (list1, (GFunc) nm_gvalue_destroy, NULL);
		g_slist_free (list1);
		g_slist_foreach (list2, (GFunc) nm_gvalue_destroy, NULL);
		g_slist_free (list2);
	}

	return ret;
}

static void
iterate_map (const GValue *key_val,
		   const GValue *value_val,
		   gpointer user_data)
{
	GHashTable **hash = (GHashTable **) user_data;

	g_hash_table_insert (*hash, g_value_dup_string (key_val), nm_gvalue_dup (value_val));
}

typedef struct {
	GHashTable *hash2;
	gint ret;
} CompareMapInfo;

static void
compare_one_map_item (gpointer key, gpointer val, gpointer user_data)
{
	CompareMapInfo *info = (CompareMapInfo *) user_data;
	GValue *value2;

	if (info->ret)
		return;

	value2 = (GValue *) g_hash_table_lookup (info->hash2, key);
	if (value2)
		info->ret = nm_gvalues_compare ((GValue *) val, value2);
	else
		info->ret = 1;
}

static gint
nm_gvalues_compare_map (const GValue *value1, const GValue *value2)
{
	GHashTable *hash1 = NULL;
	GHashTable *hash2 = NULL;
	guint len1;
	guint len2;
	gint ret = 0;

	if (dbus_g_type_get_map_key_specialization (G_VALUE_TYPE (value1)) != G_TYPE_STRING) {
		g_warning ("Can not compare maps with '%s' for keys",
				 g_type_name (dbus_g_type_get_map_key_specialization (G_VALUE_TYPE (value1))));
		return 0;
	}

	hash1 = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, nm_gvalue_destroy);
	dbus_g_type_map_value_iterate (value1, iterate_map, &hash1); 
	len1 = g_hash_table_size (hash1);

	hash2 = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, nm_gvalue_destroy);
	dbus_g_type_map_value_iterate (value2, iterate_map, &hash2);
	len2 = g_hash_table_size (hash2);

	if (len1 != len2)
		ret = len1 < len2 ? -1 : len1 > len2;
	else {
		CompareMapInfo info;

		info.ret = 0;
		info.hash2 = hash2;
		g_hash_table_foreach (hash1, compare_one_map_item, &info);
		ret = info.ret;
	}

	g_hash_table_destroy (hash1);
	g_hash_table_destroy (hash2);

	return ret;
}

static gint
nm_gvalues_compare_struct (const GValue *value1, const GValue *value2)
{
	g_warning ("Not implemented");
	return 0;
}

gint
nm_gvalues_compare (const GValue *value1, const GValue *value2)
{
	GType type1;
	GType type2;
	gint ret;

	if (value1 == value2)
		return 0;
	if (!value1)
		return 1;
	if (!value2)
		return -1;

	type1 = G_VALUE_TYPE (value1);
	type2 = G_VALUE_TYPE (value2);

	if (type1 != type2)
		return type1 < type2 ? -1 : type1 > type2;


	if (type_is_fixed_size (type1))
		ret = nm_gvalues_compare_fixed (value1, value2);
	else if (type1 == G_TYPE_STRING) 
		ret = nm_gvalues_compare_string (value1, value2);
	else if (G_VALUE_HOLDS_BOXED (value1)) {
		gpointer p1 = g_value_get_boxed (value1);
		gpointer p2 = g_value_get_boxed (value2);

		if (p1 == p2)
			ret = 0; /* Exactly the same values */
		else if (!p1)
			ret = 1; /* The comparision functions below don't handle NULLs */
		else if (!p2)
			ret = -1; /* The comparision functions below don't handle NULLs */
		else if (type1 == G_TYPE_STRV)
			ret = nm_gvalues_compare_strv (value1, value2);
		else if (dbus_g_type_is_collection (type1))
			ret = nm_gvalues_compare_collection (value1, value2);
		else if (dbus_g_type_is_map (type1))
			ret = nm_gvalues_compare_map (value1, value2);
		else if (dbus_g_type_is_struct (type1))
			ret = nm_gvalues_compare_struct (value1, value2);
		else if (type1 == G_TYPE_VALUE)
			ret = nm_gvalues_compare ((GValue *) g_value_get_boxed (value1), (GValue *) g_value_get_boxed (value2));
		else {
			g_warning ("Don't know how to compare boxed types '%s'", g_type_name (type1));
			ret = value1 == value2;
		}
	} else {
		g_warning ("Don't know how to compare types '%s'", g_type_name (type1));
		ret = value1 == value2;
	}

	return ret;
}

/***********************************************************/

static void
param_specialized_init (GParamSpec *pspec)
{
}

static void
param_specialized_set_default (GParamSpec *pspec, GValue *value)
{
	value->data[0].v_pointer = NULL;
}

static gboolean
param_specialized_validate (GParamSpec *pspec, GValue *value)
{
	NMParamSpecSpecialized *sspec = NM_PARAM_SPEC_SPECIALIZED (pspec);
	GType value_type = G_VALUE_TYPE (value);
	gboolean changed = FALSE;

	if (!g_value_type_compatible (value_type, G_PARAM_SPEC_VALUE_TYPE (sspec))) {
		g_value_reset (value);
		changed = TRUE;
	}

	return changed;
}

static gint
param_specialized_values_cmp (GParamSpec *pspec,
						const GValue *value1,
						const GValue *value2)
{
	return nm_gvalues_compare (value1, value2);
}

GType
nm_param_spec_specialized_get_type (void)
{
	static GType type;

	if (G_UNLIKELY (type) == 0) {
		static const GParamSpecTypeInfo pspec_info = {
			sizeof (NMParamSpecSpecialized),
			0,
			param_specialized_init,
			G_TYPE_OBJECT, /* value_type */
			NULL,          /* finalize */
			param_specialized_set_default,
			param_specialized_validate,
			param_specialized_values_cmp,
		};
		type = g_param_type_register_static ("NMParamSpecSpecialized", &pspec_info);
	}

	return type;
}

GParamSpec *
nm_param_spec_specialized (const char *name,
					  const char *nick,
					  const char *blurb,
					  GType specialized_type,
					  GParamFlags flags)
{
	NMParamSpecSpecialized *pspec;

	g_return_val_if_fail (g_type_is_a (specialized_type, G_TYPE_BOXED), NULL);

	pspec = g_param_spec_internal (NM_TYPE_PARAM_SPEC_SPECIALIZED,
							 name, nick, blurb, flags);

	G_PARAM_SPEC (pspec)->value_type = specialized_type;

	return G_PARAM_SPEC (pspec);
}

/***********************************************************/
/* Tests */

#if 0

static void
compare_ints (void)
{
	GValue value1 = { 0 };
	GValue value2 = { 0 };

	g_value_init (&value1, G_TYPE_INT);
	g_value_init (&value2, G_TYPE_INT);

	g_value_set_int (&value1, 5);
	g_value_set_int (&value2, 5);
	g_print ("Comparing ints 5 and 5: %d\n", nm_gvalues_compare (&value1, &value2));

	g_value_set_int (&value2, 10);
	g_print ("Comparing ints 5 and 10: %d\n", nm_gvalues_compare (&value1, &value2));

	g_value_set_int (&value2, 1);
	g_print ("Comparing ints 5 and 1: %d\n", nm_gvalues_compare (&value1, &value2));
}

static void
compare_strings (void)
{
	GValue value1 = { 0 };
	GValue value2 = { 0 };
	const char *str1 = "hello";
	const char *str2 = "world";

	g_value_init (&value1, G_TYPE_STRING);
	g_value_init (&value2, G_TYPE_STRING);

	g_value_set_string (&value1, str1);
	g_value_set_string (&value2, str1);
	g_print ("Comparing identical strings: %d\n", nm_gvalues_compare (&value1, &value2));

	g_value_set_string (&value2, str2);
	g_print ("Comparing different strings: %d\n", nm_gvalues_compare (&value1, &value2));
}

static void
compare_strv (void)
{
	GValue value1 = { 0 };
	GValue value2 = { 0 };
	char *strv1[] = { "foo", "bar", "baz", NULL };
	char *strv2[] = { "foo", "bar", "bar", NULL };
	char *strv3[] = { "foo", "bar", NULL };
	char *strv4[] = { "foo", "bar", "baz", "bam", NULL };

	g_value_init (&value1, G_TYPE_STRV);
	g_value_init (&value2, G_TYPE_STRV);

	g_value_set_boxed (&value1, strv1);
	g_value_set_boxed (&value2, strv1);
	g_print ("Comparing identical strv's: %d\n", nm_gvalues_compare (&value1, &value2));

	g_value_set_boxed (&value2, strv2);
	g_print ("Comparing different strv's: %d\n", nm_gvalues_compare (&value1, &value2));

	g_value_set_boxed (&value2, strv3);
	g_print ("Comparing different len (smaller) strv's: %d\n", nm_gvalues_compare (&value1, &value2));

	g_value_set_boxed (&value2, strv4);
	g_print ("Comparing different len (longer) strv's: %d\n", nm_gvalues_compare (&value1, &value2));
}

static void
compare_garrays (void)
{
	GArray *array1;
	GArray *array2;
	GValue value1 = { 0 };
	GValue value2 = { 0 };
	int i;

	g_value_init (&value1, DBUS_TYPE_G_UINT_ARRAY);
	array1 = g_array_new (FALSE, FALSE, sizeof (guint32));

	g_value_init (&value2, DBUS_TYPE_G_UINT_ARRAY);
	array2 = g_array_new (FALSE, FALSE, sizeof (guint32));

	for (i = 0; i < 5; i++) {
		g_array_append_val (array1, i);
		g_array_append_val (array2, i);
	}

	g_value_set_boxed (&value1, array1);
	g_value_set_boxed (&value2, array2);

	g_print ("Comparing identical arrays's: %d\n", nm_gvalues_compare (&value1, &value2));

	g_array_remove_index (array2, 0);
	g_value_set_boxed (&value2, array2);
	g_print ("Comparing different length arrays's: %d\n", nm_gvalues_compare (&value1, &value2));

	i = 7;
	g_array_prepend_val (array2, i);
	g_value_set_boxed (&value2, array2);
	g_print ("Comparing different arrays's: %d\n", nm_gvalues_compare (&value1, &value2));
}

static void
compare_ptrarrays (void)
{
	GPtrArray *array1;
	GPtrArray *array2;
	GValue value1 = { 0 };
	GValue value2 = { 0 };

	g_value_init (&value1, dbus_g_type_get_collection ("GPtrArray", G_TYPE_STRING));
	array1 = g_ptr_array_new ();

	g_value_init (&value2, dbus_g_type_get_collection ("GPtrArray", G_TYPE_STRING));
	array2 = g_ptr_array_new ();

	g_ptr_array_add (array1, "hello");
	g_ptr_array_add (array1, "world");
	g_value_set_boxed (&value1, array1);

	g_ptr_array_add (array2, "hello");
	g_ptr_array_add (array2, "world");
	g_value_set_boxed (&value2, array2);

	g_print ("Comparing identical ptr arrays's: %d\n", nm_gvalues_compare (&value1, &value2));

	g_ptr_array_add (array2, "boo");
	g_value_set_boxed (&value2, array2);
	g_print ("Comparing different len ptr arrays's: %d\n", nm_gvalues_compare (&value1, &value2));

	g_ptr_array_add (array1, "booz");
	g_value_set_boxed (&value1, array1);
	g_print ("Comparing different ptr arrays's: %d\n", nm_gvalues_compare (&value1, &value2));
}

static void
compare_str_hash (void)
{
	GHashTable *hash1;
	GHashTable *hash2;
	GValue value1 = { 0 };
	GValue value2 = { 0 };

	g_value_init (&value1, dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_STRING));
	g_value_init (&value2, dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_STRING));

	hash1 = g_hash_table_new (g_str_hash, g_str_equal);
	hash2 = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (hash1, "key1", "hello");
	g_hash_table_insert (hash1, "key2", "world");

	g_hash_table_insert (hash2, "key1", "hello");
	g_hash_table_insert (hash2, "key2", "world");

	g_value_set_boxed (&value1, hash1);
	g_value_set_boxed (&value2, hash2);
	g_print ("Comparing identical str hashes: %d\n", nm_gvalues_compare (&value1, &value2));

	g_hash_table_remove (hash2, "key2");
	g_value_set_boxed (&value2, hash2);
	g_print ("Comparing different length str hashes: %d\n", nm_gvalues_compare (&value1, &value2));

	g_hash_table_insert (hash2, "key2", "moon");
	g_value_set_boxed (&value2, hash2);
	g_print ("Comparing different str hashes: %d\n", nm_gvalues_compare (&value1, &value2));
}

static GValue *
str_to_gvalue (const char *str)
{
	GValue *value;

	value = g_slice_new0 (GValue);
	g_value_init (value, G_TYPE_STRING);
	g_value_set_string (value, str);

	return value;
}

static GValue *
int_to_gvalue (int i)
{
	GValue *value;

	value = g_slice_new0 (GValue);
	g_value_init (value, G_TYPE_INT);
	g_value_set_int (value, i);

	return value;
}

static void
compare_gvalue_hash (void)
{
	GHashTable *hash1;
	GHashTable *hash2;
	GValue value1 = { 0 };
	GValue value2 = { 0 };

	g_value_init (&value1, dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_VALUE));
	g_value_init (&value2, dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_VALUE));

	hash1 = g_hash_table_new (g_str_hash, g_str_equal);
	hash2 = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (hash1, "key1", str_to_gvalue ("hello"));
	g_hash_table_insert (hash1, "key2", int_to_gvalue (5));

	g_hash_table_insert (hash2, "key1", str_to_gvalue ("hello"));
	g_hash_table_insert (hash2, "key2", int_to_gvalue (5));

	g_value_set_boxed (&value1, hash1);
	g_value_set_boxed (&value2, hash2);
	g_print ("Comparing identical gvalue hashes: %d\n", nm_gvalues_compare (&value1, &value2));

	g_hash_table_remove (hash2, "key2");
	g_value_set_boxed (&value2, hash2);
	g_print ("Comparing different length str hashes: %d\n", nm_gvalues_compare (&value1, &value2));

	g_hash_table_insert (hash2, "key2", str_to_gvalue ("moon"));
	g_value_set_boxed (&value2, hash2);
	g_print ("Comparing different str hashes: %d\n", nm_gvalues_compare (&value1, &value2));
}

int
main (int argc, char *argv[])
{
	DBusGConnection *bus;

	g_type_init ();
	bus = dbus_g_bus_get (DBUS_BUS_SESSION, NULL);

	compare_ints ();
	compare_strings ();
	compare_strv ();
	compare_garrays ();
	compare_ptrarrays ();
	compare_str_hash ();
	compare_gvalue_hash ();

	return 0;
}

#endif

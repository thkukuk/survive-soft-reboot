
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>

#include <libeconf.h>
#include <libmount/libmount.h>

/*
  TU_SETUP means, we have a read-only root filesystem managed with
  transactional-update and we start from the root subvolume.
  TW_SETUP means, we have a traditional read-write root filesystem
  managed by zypper and snapper and admin creates an extra snapshot,
  from which we start the application.
*/
#define TU_SETUP 1
#define TW_SETUP 2

static char
hexchar (int x)
{
  static const char table[] = "0123456789abcdef";

  return table[x & 15];
}

static char *
escape_char (char src, char *dst)
{
  *(dst++) = '\\';
  *(dst++) = 'x';
  *(dst++) = hexchar(src >> 4);
  *(dst++) = hexchar(src);

  return dst;
}

#define VALID_CHARS \
  "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ:-_.\\"

static char *
escape_str (const char *src, char *dst)
{
  char *cp = dst;

  for (; *src; src++)
    {
      if (*src == '/')
	*(cp++) = '-';
      else
	{
	  if (*src == '-' || *src == '\\' || !strchr(VALID_CHARS, *src))
	    cp = escape_char (*src, cp);
	  else
	    *(cp++) = *src;
	}
    }

  *(cp++) = '\0';

  return dst;
}

static int
determine_mode (const char *product_name)
{
  if (strcmp (product_name, "openSUSE MicroOS") == 0)
    return TU_SETUP;

  if (strcmp (product_name, "openSUSE Tumbleweed") == 0)
    return TW_SETUP;

  return -1;
}

static char *
get_product_name (void)
{
  /* get OS informations */
  econf_file *econf = NULL;
  econf_err error;

  if (econf_readFile (&econf, "/etc/os-release", "=", "#"))
    {
      if ((error = econf_readFile (&econf, "/usr/lib/os-release", "=", "#")))
	{
	  fprintf (stderr, "ERROR: couldn't read os-release: %s\n",
		   econf_errString (error));
	  return NULL;
	}
    }

  char *product_name = NULL;
  if ((error = econf_getStringValue (econf, "", "NAME",
                                     &product_name)))
    {
      fprintf (stderr,
               "ERROR: couldn't read \"NAME\" from os-release: %s\n",
               econf_errString(error));
      return NULL;
    }
  econf_free (econf);

  return product_name;
}

static int
mkdir_p (const char *path)
{
  int ret = 0;

  if (!path || !*path)
    return -EINVAL;

  char *dir = strdup (path);
  if (!dir)
    return -ENOMEM;
  char *p = dir;

  if (*p == '/')
    p++;

  while (p && *p)
    {
      char *cp = strchr (p, '/');
      if (cp)
	*cp = '\0';
      if (*p)
	{
	  ret = mkdir (dir, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	  if (ret && errno != EEXIST)
	    break;
	  ret = 0;
	}
      if (!cp)
	break;
      *cp = '/';
      p = cp + 1;
    }

  free (dir);
  return ret;
}

static int
create_file (const char *name, const char *content)
{
  FILE *fp = fopen (name, "w");
  if (fp == NULL)
    {
      fprintf (stderr, "ERROR, cannot create %s: %m\n", name);
      return -1;
    }
  if (fprintf (fp, "%s", content) < 0)
    {
      fprintf (stderr, "ERROR writing %s: %m\n", name);
      fclose (fp);
      return -1;
    }
  fclose (fp);

  return 0;
}

static int
add_to_file (const char *name, const char *content)
{
  FILE *fp = fopen (name, "a");
  if (fp == NULL)
    {
      fprintf (stderr, "ERROR, cannot create %s: %m\n", name);
      return -1;
    }
  if (fprintf (fp, "%s", content) < 0)
    {
      fprintf (stderr, "ERROR writing %s: %m\n", name);
      fclose (fp);
      return -1;
    }
  fclose (fp);

  return 0;
}

static int
create_unit (const char *service, econf_file *key_file,
	     const char *device, int64_t subvolid,
	     const char *generator_dir, int mode)
{
  int is_templated = 0;
  char *service_d_dir = NULL;
  char *service_snippet = NULL;
  char *content = NULL;
  int64_t subvolid_config = -1;
  int64_t snapshot_config = -1;
  int retval;
  econf_err error;

  if ((error = econf_getInt64Value (key_file, service, "_subvolid", &subvolid_config)))
    {
      if (error != ECONF_NOKEY)
	{
	  fprintf (stderr, "Error reading value of \"_subvolid\": %s\n",
		   econf_errString(error));
	  return -1;
	}
    }

  if ((error = econf_getInt64Value (key_file, service, "_snapshot", &snapshot_config)))
    {
      if (error != ECONF_NOKEY)
	{
	  fprintf (stderr, "Error reading value of \"_snapshot\": %s\n",
		   econf_errString(error));
	  return -1;
	}
    }

  if (mode == TW_SETUP && subvolid_config == -1 && snapshot_config == -1)
    {
      fprintf (stderr, "ERROR: %s: read-write system but no subvolid nor snapshot number provided!\n",
	       service);
      return -1;
    }

  if (service[strlen (service) -1] == '@')
    is_templated = 1;

  if (asprintf (&service_d_dir, "%s/%s.service.d", generator_dir, service) < 0)
    {
      fprintf (stderr, "ERROR: out of memory!\n");
      return -1;
    }

  if (mkdir_p (service_d_dir) < 0)
    {
      fprintf (stderr, "ERROR: cannot create \"%s\": %m\n", service_d_dir);
      return -1;
    }

  // Create default dependencies for the unit which prevents systemd on
  // soft-reboot of killing it.
  if (asprintf (&service_snippet, "%s/dependencies.conf", service_d_dir) < 0)
    {
      fprintf (stderr, "ERROR: out of memory!\n");
      return -1;
    }

  retval = create_file (service_snippet,
			"[Unit]\n"
			"SurviveFinalKillSignal=yes\n"
			"IgnoreOnIsolate=yes\n"
			"DefaultDependencies=no\n"
			"After=basic.target\n"
			"Conflicts=reboot.target kexec.target poweroff.target halt.target rescue.target emergency.target\n"
			"Before=shutdown.target rescue.target emergency.target\n"
			"\n"
			"[Service]\n"
			"TemporaryFileSystem=/var\n"
			"TemporaryFileSystem=/tmp\n"
                        "BindReadOnlyPaths=/run/systemd/journal /run/systemd/journal/stdout\n"
                        "BindReadOnlyPaths=/run/dbus/system_bus_socket\n"
			);

  if (mode == TU_SETUP && retval == 0)
    retval = add_to_file (service_snippet, "BindReadOnlyPaths=/etc\n");

  if (retval == 0)
    {
        char **keys;
	size_t key_number;

        error = econf_getKeys (key_file, service, &key_number, &keys);
	if (error)
	  {
	    fprintf (stderr, "Error getting all keys: %s\n",
		     econf_errString(error));
	    retval = -1;
	  }
	else
	  {
	    for (size_t i = 0; i < key_number; i++)
	      {
		char *val;
		char *content;

		// keys starting with "_" are internal
		if (keys[i][0] == '_')
		  continue;

		if ((error = econf_getStringValue (key_file, service,
						   keys[i], &val)))
		  {
		    fprintf (stderr, "Error reading [%s]: %s\n",
			     keys[i], econf_errString(error));
		    retval = -1;
		  }

		if (asprintf (&content, "%s=%s\n", keys[i], val) < 0)
		  {
		    fprintf (stderr, "ERROR: out of memory!\n");
		    return -1;
		  }
		/* XXX this is slow... */
		add_to_file (service_snippet, content);
		free (content);
		free (val);
	      }
	    econf_free (keys);
	  }
    }

  free (service_snippet);
  if (retval < 0)
    return -1;

  // Create snippet with RootImage and RootImageOptions
  if (asprintf (&service_snippet, "%s/rootimage.conf", service_d_dir) < 0)
    {
      fprintf (stderr, "ERROR: out of memory!\n");
      return -1;
    }

  if (is_templated)
    {
      // We have something like unit@<btrfs-id>.service
      // Create a slice for it.
      char *slice;
      char str[strlen (service) + 1];
      char escaped[strlen (service) * 4 + 1];

      char *cp = stpcpy (str, service);
      *(--cp) = '\0';

      if (asprintf (&slice, "%s/system-%s.slice", generator_dir, escape_str (str, escaped)) < 0)
	{
	  fprintf (stderr, "ERROR: out of memory!\n");
	  return -1;
	}

      if (asprintf (&content, "[Unit]\n"
		    "Description=Slice for %s.service template\n"
		    "SurviveFinalKillSignal=yes\n"
		    "IgnoreOnIsolate=yes\n"
		    "DefaultDependencies=no\n", service) < 0)
	{
	  free (slice);
	  fprintf (stderr, "ERROR: out of memory!\n");
	  return -1;
	}

      retval = create_file (slice, content);
      free (slice);
      free (content);
      if (retval < 0)
	return -1;
    }

  if (snapshot_config != -1)
    {
      if (asprintf (&content, "[Service]\n"
		    "RootImage=%s\n"
		    "RootImageOptions=ro,subvol=/@/.snapshots/%li/snapshot/\n",
		    device, snapshot_config) < 0)
	{
	  free (service_snippet);
	  fprintf (stderr, "ERROR: out of memory!\n");
	  return -1;
	}
    }
  else
    {
      if (asprintf (&content, "[Service]\n"
		    "RootImage=%s\n"
		    "RootImageOptions=ro,subvolid=%ld\n",
		    device, subvolid_config==-1?subvolid:subvolid_config) < 0)
	{
	  free (service_snippet);
	  fprintf (stderr, "ERROR: out of memory!\n");
	  return -1;
	}
    }

  retval = create_file (service_snippet, content);
  free (service_snippet);
  free (content);
  if (retval < 0)
    return -1;

  free (service_d_dir);

  return 0;
}

int
main (int argc, char **argv)
{
  while (1)
    {
      int c;
      int option_index = 0;
      static struct option long_options[] =
      {
        {"version", no_argument, NULL, '\255'},
        {"usage", no_argument, NULL, '\254'},
        {"help", no_argument, NULL, '?'},
        {NULL, 0, NULL, '\0'}
      };

      c = getopt_long (argc, argv, "?", long_options, &option_index);
      if (c == (-1))
        break;
      switch (c)
        {
        case '?':
          // print_help ();
          return 0;
        case '\255':
          // print_version ();
          return 0;
        case '\254':
          // print_usage (stdout);
          return 0;
        default:
          // print_usage (stderr);
          return 1;
        }
    }

  argc -= optind;
  argv += optind;

  if (argc < 1)
    {
      fprintf (stderr, "Usage: btrfs-soft-reboot-generator <path>\n");
      return 1;
    }
  const char *generator_dir = argv[0];

  char *product_name = get_product_name ();
  if (product_name == NULL)
    return 1;

  int mode = determine_mode (product_name);
  if (mode < 0)
    {
      fprintf (stderr, "ERROR: \"%s\" is not supported!\n", product_name);
      return 1;
    }
  free (product_name);

  /* initialize libmount */
  mnt_init_debug(0);

  struct libmnt_table *tb;
  int ret = 0;

  tb = mnt_new_table ();
  if (!tb)
    {
      fprintf (stderr, "failed to initialize libmount table");
      return 1;
    }

  ret = mnt_table_parse_mtab(tb, NULL);
#if 0
  XXX parse first kernel, try mtab as fallback
  case TABTYPE_KERNEL:
    if (!path)
      path = access(_PATH_PROC_MOUNTINFO, R_OK) == 0 ?
	_PATH_PROC_MOUNTINFO :
	_PATH_PROC_MOUNTS;

	ret = mnt_table_parse_file(tb, path);
	break;
#endif
  if (ret)
    {
      mnt_unref_table (tb);
      fprintf (stderr, "ERROR: can't read mtab.");
      return 1;
    }

  struct libmnt_fs *fs = mnt_table_find_target (tb, "/", MNT_ITER_FORWARD);
  if (fs == NULL)
    {
      mnt_unref_table (tb);
      fprintf (stderr, "ERROR: can't find mount entry for \"/\".");
      return 1;
    }

  const char *fstype = mnt_fs_get_fstype (fs);
  if (strcmp (fstype, "btrfs") != 0)
    {
      fprintf (stderr, "ERROR: filesystem type \"%s\" is not supported!\n",
	       fstype);
      return 1;
    }

  char *subvolidstr = NULL;
  size_t subvolidsz = 0;
  int64_t subvolid = -1;
  if (mnt_fs_get_option (fs, "subvolid", &subvolidstr, &subvolidsz) == 0)
    {
      subvolidstr = strndup (subvolidstr, subvolidsz);
      subvolid = atoi (subvolidstr); /* XXX error handling */
      free (subvolidstr);
    }

  econf_file *key_file = NULL;
  econf_err error = econf_readConfig (&key_file,
				      NULL,
				      "/usr/etc",
				      "btrfs-soft-reboot", "conf", "=", "#");
  if (error)
    {
      if (error != ECONF_NOFILE)
	{
	  fprintf (stderr, "ERROR: btrfs-soft-reboot: %s\n",
		   econf_errString (error));
	  return 1;
	}
      else
	{
	  /* no config, nothing to do */
	  return 0;
	}
    }

  char **services;
  size_t service_number;
  if ((error = econf_getGroups (key_file, &service_number, &services)))
    {
      fprintf (stderr, "Error getting all services: %s\n",
	       econf_errString (error));
      return 1;
    }
  if (service_number == 0)
    {
      fprintf (stdout, "No services found...\n");
      return 0;
    }

  int retval = 0;
  for (size_t i = 0; i < service_number; i++)
    if (create_unit (services[i], key_file, mnt_fs_get_srcpath (fs), subvolid, generator_dir, mode) != 0)
      retval = 1;

  mnt_unref_fs (fs);
  mnt_free_table (tb);

  return retval;
}

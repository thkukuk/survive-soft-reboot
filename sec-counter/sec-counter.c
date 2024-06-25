
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#include <systemd/sd-daemon.h>
#include <systemd/sd-journal.h>

int
main (int argc, char **argv)
{
  unsigned long counter = 0;
  int journal = 0;
  int outerr = 0;

  // signal(SIGPIPE, SIG_DFL);

  while (1)
    {
      int c;
      int option_index = 0;
      static struct option long_options[] =
      {
        {"version", no_argument, NULL, '\255'},
        {"usage", no_argument, NULL, '\254'},
        {"help", no_argument, NULL, '?'},
	{"stderr", no_argument, NULL, 's'},
	{"journal", no_argument, NULL, 'j'},
        {NULL, 0, NULL, '\0'}
      };

      c = getopt_long (argc, argv, "js?", long_options, &option_index);
      if (c == (-1))
        break;
      switch (c)
        {
	case 'j':
	  journal = 1;
	  break;
	case 's':
	  outerr = 1;
	  break;
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

  if ((journal + outerr) == 0)
    journal = 1;

  sd_notify (0, "READY=1");

  while (1)
    {
      sleep (1);
      counter++;

      if (journal)
	{
	  int ret = sd_journal_print (LOG_INFO, "Journal-Counter: %li seconds", counter);
	  // during restart of journald we get ECONNREFUSED
	  if (ret < 0 && ret != -ECONNREFUSED)
	    return -ret;
	}
      if (outerr)
	{
	  int ret = fprintf (stderr, "STDERR-Counter: %li seconds\n", counter);
	  if (ret <= 0)
	    {
	      sd_journal_print (LOG_ERR, "Couldn't write to stderr: %m");
	      // return -ret;
	    }
	}
    }

  return 0;
}

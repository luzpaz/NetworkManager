/* NetworkManager -- Network link manager
 *
 * Narayan Newton <narayan_newton@yahoo.com>
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
 *
 * (C) Copyright 2004 RedHat, Inc.
 * (C) Copyright 2004 Narayan Newton
 */

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include "NetworkManagerSystem.h"
#include "NetworkManagerUtils.h"
#include "NetworkManagerDevice.h"

/*
 *	Mostly a mix of the Gentoo and RedHat Backends
 */

/*
 * nm_system_init
 *
 * Initializes the distribution-specific system backend
 *
 */
void nm_system_init (void)
{
}

/*
 * nm_system_device_run_dhcp
 *
 * Run the dhcp daemon for a particular interface.
 *
 * Returns:	TRUE on success
 *			FALSE on dhcp error
 *
 */
gboolean nm_system_device_run_dhcp (NMDevice *dev)
{
        char             buf [500];
        int              err;

        g_return_val_if_fail (dev != NULL, FALSE);

        /* Fake it for a test device */
        if (nm_device_is_test_device (dev))
        {
                g_usleep (2000);
                return (TRUE);
        }

        snprintf (buf, 500, "/sbin/dhcpcd %s", nm_device_get_iface(dev));
        err = nm_spawn_process (buf);
        return (err == 0);
}



/*
 * nm_system_device_stop_dhcp
 *
 * Kill any dhcp daemon that happens to be around.  We may be changing
 * interfaces and we're going to bring the previous one down, so there's
 * no sense in keeping the dhcp daemon running on the old interface.
 *
 */
void nm_system_device_stop_dhcp (NMDevice *dev)
{
	FILE			*pidfile;
	char			 buf [500];

	g_return_if_fail (dev != NULL);

	/* Not really applicable for test devices */
	if (nm_device_is_test_device (dev))
		return;

	/* Find and kill the previous dhclient process for this device */
	snprintf (buf, 500, "/etc/dhcpc/dhcpcd-%s.pid", nm_device_get_iface (dev));
	pidfile = fopen (buf, "r");
	if (pidfile)
	{
		int			len;
		unsigned char	s_pid[20];
		pid_t		n_pid = -1;

		memset (s_pid, 0, 20);
		fgets (s_pid, 20, pidfile);
		len = strlen (s_pid);
		fclose (pidfile);

		n_pid = atoi (s_pid);
		if (n_pid > 0)
			kill (n_pid, SIGTERM);
	}
}


/*
 * nm_system_device_flush_routes
 *
 * Flush all routes associated with a network device
 *
 */
void nm_system_device_flush_routes (NMDevice *dev)
{
	char	buf [100];

	g_return_if_fail (dev != NULL);

	/* Not really applicable for test devices */
	if (nm_device_is_test_device (dev))
		return;

	/* Remove routing table entries */
	snprintf (buf, 100, "/sbin/ip route flush dev %s", nm_device_get_iface (dev));
	nm_spawn_process (buf);
}


/*
 * nm_system_device_flush_addresses
 *
 * Flush all network addresses associated with a network device
 *
 */
void nm_system_device_flush_addresses (NMDevice *dev)
{
	char	buf [100];

	g_return_if_fail (dev != NULL);

	/* Not really applicable for test devices */
	if (nm_device_is_test_device (dev))
		return;

	/* Remove all IP addresses for a device */
	snprintf (buf, 100, "/sbin/ip address flush dev %s", nm_device_get_iface (dev));
	nm_spawn_process (buf);
}


/*
 * nm_system_device_update_config_info
 *
 * Retrieve any relevant configuration info for a particular device
 * from the system network configuration information.  Clear out existing
 * info before setting stuff too.
 *
 */
void nm_system_device_update_config_info (NMDevice *dev)
{
}


/*
 * nm_system_enable_loopback
 *
 * Bring up the loopback interface
 *
 */
void nm_system_enable_loopback (void)
{
	nm_spawn_process ("/sbin/ip link set dev lo up");
	nm_spawn_process ("/sbin/ip addr add 127.0.0.1/8 brd 127.255.255.255 dev lo label loopback");
}


/*
 * nm_system_delete_default_route
 *
 * Remove the old default route in preparation for a new one
 *
 */
void nm_system_delete_default_route (void)
{
	nm_spawn_process ("/sbin/ip route del default");
}


/*
 * nm_system_kill_all_dhcp_daemons
 *
 * Kill all DHCP daemons currently running, done at startup.
 *
 */
void nm_system_kill_all_dhcp_daemons (void)
{
	nm_spawn_process ("/usr/bin/killall -q dhcpcd");
}


/*
 * nm_system_update_dns
 *
 * Make glibc/nscd aware of any changes to the resolv.conf file by
 * restarting nscd.
 *
 */
void nm_system_update_dns (void)
{
}


/*
 * nm_system_load_device_modules
 *
 * These should already be loaded 
 *
 */
void nm_system_load_device_modules (void)
{
}




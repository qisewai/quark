
#include <net/if.h>

unsigned int if_nametoindex (const char *ifname)
{
  struct ifreq ifr;
  int fd = __opensock ();

  if(fd < 0)
    return 0;

  if (strlen (ifname) >= IFNAMSIZ)
  {
      set_errno (ENODEV);
      return 0;
  }

  strncpy (ifr.ifr_name, ifname, sizeof (ifr.ifr_name));
  if (__ioctl(fd, SIOCGIFINDEX, &ifr) < 0)
  {
      int saved_errno = errno;
      __close_nocancel_nostatus (fd);
      if (saved_errno == EINVAL)
	__set_errno (ENOSYS);
      return 0;
  }
  __close_nocancel_nostatus (fd);
  return ifr.ifr_ifindex;
}


void if_freenameindex (struct if_nameindex *ifn)
{
  struct if_nameindex *ptr = ifn;
  while (ptr->if_name || ptr->if_index)
  {
      free (ptr->if_name);
      ++ptr;
  }
  free (ifn);
}

static struct if_nameindex * if_nameindex_netlink (void)
{
  struct netlink_handle nh = { 0, 0, 0, NULL, NULL };
  struct if_nameindex *idx = NULL;

  if (__netlink_open (&nh) < 0)
    return NULL;


  /* Tell the kernel that we wish to get a list of all
     active interfaces.  Collect all data for every interface.  */
  if (__netlink_request (&nh, RTM_GETLINK) < 0)
    goto exit_free;

  /* Count the interfaces.  */
  unsigned int nifs = 0;
  for (struct netlink_res *nlp = nh.nlm_list; nlp; nlp = nlp->next)
    {
      struct nlmsghdr *nlh;
      size_t size = nlp->size;

      if (nlp->nlh == NULL)
	continue;

      /* Walk through all entries we got from the kernel and look, which
         message type they contain.  */
      for (nlh = nlp->nlh; NLMSG_OK (nlh, size); nlh = NLMSG_NEXT (nlh, size))
	{
	  /* Check if the message is what we want.  */
	  if ((pid_t) nlh->nlmsg_pid != nh.pid || nlh->nlmsg_seq != nlp->seq)
	    continue;

	  if (nlh->nlmsg_type == NLMSG_DONE)
	    break;		/* ok */

	  if (nlh->nlmsg_type == RTM_NEWLINK)
	    ++nifs;
	}
    }

  idx = malloc ((nifs + 1) * sizeof (struct if_nameindex));
  if (idx == NULL)
    {
    nomem:
      __set_errno (ENOBUFS);
      goto exit_free;
    }

  /* Add the interfaces.  */
  nifs = 0;
  for (struct netlink_res *nlp = nh.nlm_list; nlp; nlp = nlp->next)
    {
      struct nlmsghdr *nlh;
      size_t size = nlp->size;

      if (nlp->nlh == NULL)
	continue;

      /* Walk through all entries we got from the kernel and look, which
         message type they contain.  */
      for (nlh = nlp->nlh; NLMSG_OK (nlh, size); nlh = NLMSG_NEXT (nlh, size))
	{
	  /* Check if the message is what we want.  */
	  if ((pid_t) nlh->nlmsg_pid != nh.pid || nlh->nlmsg_seq != nlp->seq)
	    continue;

	  if (nlh->nlmsg_type == NLMSG_DONE)
	    break;		/* ok */

	  if (nlh->nlmsg_type == RTM_NEWLINK)
	    {
	      struct ifinfomsg *ifim = (struct ifinfomsg *) NLMSG_DATA (nlh);
	      struct rtattr *rta = IFLA_RTA (ifim);
	      size_t rtasize = IFLA_PAYLOAD (nlh);

	      idx[nifs].if_index = ifim->ifi_index;

	      while (RTA_OK (rta, rtasize))
		{
		  char *rta_data = RTA_DATA (rta);
		  size_t rta_payload = RTA_PAYLOAD (rta);

		  if (rta->rta_type == IFLA_IFNAME)
		    {
		      idx[nifs].if_name = __strndup (rta_data, rta_payload);
		      if (idx[nifs].if_name == NULL)
			{
			  idx[nifs].if_index = 0;
			  __if_freenameindex (idx);
			  idx = NULL;
			  goto nomem;
			}
		      break;
		    }

		  rta = RTA_NEXT (rta, rtasize);
		}

	      ++nifs;
	    }
	}
    }

  idx[nifs].if_index = 0;
  idx[nifs].if_name = NULL;

 exit_free:
  __netlink_free_handle (&nh);
  __netlink_close (&nh);

  return idx;
}


struct if_nameindex * if_nameindex (void)
{
  struct if_nameindex *result = if_nameindex_netlink ();
  return result;
}

char * if_indextoname (unsigned int ifindex, char *ifname)
{
  /* We may be able to do the conversion directly, rather than searching a
     list.  This ioctl is not present in kernels before version 2.1.50.  */
  struct ifreq ifr;
  int fd;
  int status;

  fd = __opensock ();

  if (fd < 0)
    return NULL;

  ifr.ifr_ifindex = ifindex;
  status = __ioctl (fd, SIOCGIFNAME, &ifr);

  __close_nocancel_nostatus (fd);

  if (status  < 0)
    {
      if (errno == ENODEV)
	/* POSIX requires ENXIO.  */
	__set_errno (ENXIO);

      return NULL;
    }
  else
    return strncpy (ifname, ifr.ifr_name, IFNAMSIZ);
}


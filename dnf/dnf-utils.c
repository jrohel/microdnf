/* dnf-utils.c
 *
 * Copyright © 2010-2015 Richard Hughes <richard@hughsie.com>
 * Copyright © 2016 Colin Walters <walters@verbum.org>
 * Copyright © 2016-2017 Igor Gnatenko <ignatenko@redhat.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dnf-utils.h"
#include <libsmartcols.h>


// transaction details columns
enum { COL_NEVRA, COL_REPO, COL_SIZE };


static gint
dnf_package_cmp_cb (DnfPackage **pkg1, DnfPackage **pkg2)
{
  return dnf_package_cmp (*pkg1, *pkg2);
}


static void
dnf_utils_add_transaction_packages (struct libscols_table *tb,
                                    struct libscols_line *parent,
                                    GPtrArray *pkgs)
{
  // sort packages by NEVRA
  g_ptr_array_sort (pkgs, (GCompareFunc) dnf_package_cmp_cb);

  for (guint i = 0; i < pkgs->len; i++)
    {
      DnfPackage *pkg = pkgs->pdata[i];

      struct libscols_line *ln = scols_table_new_line (tb, parent);
      scols_line_set_data (ln, COL_NEVRA, dnf_package_get_nevra (pkg));
      scols_line_set_data (ln, COL_REPO, dnf_package_get_reponame (pkg));
      scols_line_set_data (ln, COL_SIZE, g_format_size (dnf_package_get_size (pkg)));
    }
}


gboolean
dnf_utils_print_transaction (DnfContext *ctx)
{
  g_autoptr(GPtrArray) pkgs = dnf_goal_get_packages (dnf_context_get_goal (ctx),
                                                     DNF_PACKAGE_INFO_INSTALL,
                                                     DNF_PACKAGE_INFO_REINSTALL,
                                                     DNF_PACKAGE_INFO_DOWNGRADE,
                                                     DNF_PACKAGE_INFO_UPDATE,
                                                     DNF_PACKAGE_INFO_REMOVE,
                                                     -1);

  if (pkgs->len == 0)
    {
      g_print ("Nothing to do.\n");
      return FALSE;
    }

  struct libscols_line *ln;

  struct libscols_table *tb = scols_new_table ();
  scols_table_new_column (tb, "Package",    0.7, SCOLS_FL_TREE);
  scols_table_new_column (tb, "Repository", 0.2, SCOLS_FL_TRUNC);
  scols_table_new_column (tb, "Size",       0.1, SCOLS_FL_RIGHT);
  scols_table_enable_maxout (tb, 1);
  struct libscols_symbols *sb = scols_new_symbols ();
  scols_symbols_set_branch (sb, " ");
  scols_symbols_set_right (sb, " ");
  scols_symbols_set_vertical (sb, " ");
  scols_table_set_symbols (tb, sb);

  g_autoptr(GPtrArray) pkgs_install = dnf_goal_get_packages (dnf_context_get_goal (ctx),
                                                             DNF_PACKAGE_INFO_INSTALL,
                                                             -1);
  if (pkgs_install->len != 0)
    {
      ln = scols_table_new_line (tb, NULL);
      scols_line_set_data (ln, COL_NEVRA, "Installing:");
      dnf_utils_add_transaction_packages (tb, ln, pkgs_install);
    }


  g_autoptr(GPtrArray) pkgs_reinstall = dnf_goal_get_packages (dnf_context_get_goal (ctx),
                                                               DNF_PACKAGE_INFO_REINSTALL,
                                                               -1);
  if (pkgs_reinstall->len != 0)
    {
      ln = scols_table_new_line (tb, NULL);
      scols_line_set_data (ln, COL_NEVRA, "Reinstalling:");
      dnf_utils_add_transaction_packages (tb, ln, pkgs_reinstall);
    }

  g_autoptr(GPtrArray) pkgs_upgrade = dnf_goal_get_packages (dnf_context_get_goal (ctx),
                                                             DNF_PACKAGE_INFO_UPDATE,
                                                             -1);
  if (pkgs_upgrade->len != 0)
    {
      ln = scols_table_new_line (tb, NULL);
      scols_line_set_data (ln, COL_NEVRA, "Upgrading:");
      dnf_utils_add_transaction_packages (tb, ln, pkgs_upgrade);
    }

  g_autoptr(GPtrArray) pkgs_remove = dnf_goal_get_packages (dnf_context_get_goal (ctx),
                                                            DNF_PACKAGE_INFO_REMOVE,
                                                            -1);
  if (pkgs_remove->len != 0)
    {
      ln = scols_table_new_line (tb, NULL);
      scols_line_set_data (ln, COL_NEVRA, "Removing:");
      dnf_utils_add_transaction_packages (tb, ln, pkgs_remove);
    }

  g_autoptr(GPtrArray) pkgs_downgrade = dnf_goal_get_packages (dnf_context_get_goal (ctx),
                                                               DNF_PACKAGE_INFO_DOWNGRADE,
                                                               -1);
  if (pkgs_downgrade->len != 0)
    {
      ln = scols_table_new_line (tb, NULL);
      scols_line_set_data (ln, COL_NEVRA, "Downgrading:");
      dnf_utils_add_transaction_packages (tb, ln, pkgs_downgrade);
    }

  scols_print_table (tb);
  scols_unref_table (tb);

  g_print ("Transaction Summary:\n");
  g_print (" %-15s %4d packages\n", "Installing:", pkgs_install->len);
  g_print (" %-15s %4d packages\n", "Reinstalling:", pkgs_reinstall->len);
  g_print (" %-15s %4d packages\n", "Upgrading:", pkgs_upgrade->len);
  g_print (" %-15s %4d packages\n", "Removing:", pkgs_remove->len);
  g_print (" %-15s %4d packages\n", "Downgrading:", pkgs_downgrade->len);

  return TRUE;
}

/* Copyright 2012-present Facebook, Inc.
 * Licensed under the Apache License, Version 2.0 */

#include "watchman.h"

// The ignore logic is to stop recursion of grandchildren or later
// generations than an ignored dir.  We allow the direct children
// of an ignored dir, but no further down.
bool w_is_ignored(w_root_t *root, const char *path, uint32_t pathlen)
{
  w_ht_iter_t i;

  if (w_ht_first(root->ignore_dirs, &i)) do {
    w_string_t *ign = w_ht_val_ptr(i.value);

    if (pathlen < ign->len) {
      continue;
    }

    if (memcmp(ign->buf, path, ign->len) == 0) {
      if (ign->len == pathlen) {
        // Exact match
        return true;
      }

      if (path[ign->len] == WATCHMAN_DIR_SEP) {
        // prefix match
        return true;
      }
    }

  } while (w_ht_next(root->ignore_dirs, &i));

  if (w_ht_first(root->ignore_vcs, &i)) do {
    w_string_t *ign = w_ht_val_ptr(i.value);

    if (pathlen <= ign->len) {
      continue;
    }

    if (memcmp(ign->buf, path, ign->len) == 0) {
      // prefix matches, but it isn't a parent
      if (path[ign->len] != WATCHMAN_DIR_SEP) {
        continue;
      }

      // If we find any '/' in the remainder of the path, then we should
      // ignore it.  Otherwise we allow it.
      path += ign->len + 1;
      pathlen -= ign->len + 1;
      if (memchr(path, WATCHMAN_DIR_SEP, pathlen)) {
        return true;
      }
    }

  } while (w_ht_next(root->ignore_vcs, &i));

  return false;
}

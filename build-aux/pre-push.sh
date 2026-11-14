#! /bin/sh
# Installed by 'guix git authenticate'.
# Amended by GNU Mes
set -e

branch=$(git branch --show-current)
wip_p=$(echo $branch | grep ^wip ||:)
if [ -n "$wip_p" ]; then
    echo "skipping authentication for WIP branch..." 1>&2
    # No need to authenticate wip-* branches
    exit
fi
release_p=$(echo $branch | grep -E '^master|version-' ||:)
draft_p=$(git log --oneline $branch | grep -E '^(DRAFT|HACK|REMOVEME|TODO|WIP|fixup|squash|wip)' ||:)
if [ -n "$draft_p" ]; then
    if [ -n "$release_p" ]; then
        echo "cowardly refusing to push $branch with DRAFT/WIP commits." 1>&2
        exit 1
    fi
    echo "skipping authentication for branch with DRAFT/WIP commits..." 1>&2
    # No need to authenticate branches with DRAFT commits
    exit
fi

# The "empty hash" used by Git when pushing a branch deletion.
z40=0000000000000000000000000000000000000000

while read local_ref local_oid remote_ref remote_oid
do
  if [ "$local_oid" != "$z40" ]
  then
      guix git authenticate --end="$local_oid"
  fi
done

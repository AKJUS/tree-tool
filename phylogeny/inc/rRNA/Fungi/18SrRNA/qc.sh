#!/bin/bash --noprofile
source CPP_DIR/bash_common.sh
if [ $# -ne 1 ]; then
  echo "Quality control of distTree_inc_new.sh"
  echo "#1: verbose (0/1)"
  echo "Requires: Time: O(n log^3(n))"
  exit 1
fi
VERB=$1


if [ $VERB == 1 ]; then
  set -x
fi


INC=`dirname $0`
CPP_DIR/phylogeny/database/LocusQC.sh $INC "Locus" "id" 4751 "18S" $VERB


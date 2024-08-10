#!/bin/bash --noprofile
source CPP_DIR/bash_common.sh
if [ $# -ne 4 ]; then
  echo "Compute dissimilarities for pairs of objects"
  echo "#1: input dissimilarity requests (pairs of objects)"
  echo "#2: new object file or directory, or ''. Object name is basename"
  echo "#3: output dissimilarities added to the pairs of objects"
  echo "#4: error log"
  exit 1
fi
REQ=$1
FILE_NEW="$2"
OUT=$3
LOG=$4


if [ $FILE_NEW ]; then
  error "$0 is not implemented"
fi


INC=`dirname $0`
GENOME=$INC/../genome

TMP=`mktemp`

awk '{printf "'$GENOME'/%s/%s.prot-univ '$GENOME'/%s/%s.prot-univ\n", $1, $1, $2, $2};' $REQ > $TMP
CPP_DIR/dissim/prot_collection2dissim  $INC/hmm-univ.stat $TMP $TMP.res  -blosum62  -raw_power 0.59  -coeff 60  -log $LOG

cut -f 3 $TMP.res > $TMP.dissim
paste $REQ $TMP.dissim > $OUT

rm $TMP*

rm -f $LOG



#CPP_DIR/phylogeny/database/combine_dissims.sh $REQ $GENOME "$FILE_NEW" $OUT 10 0.1 $INC/dissim_scale $INC/hmm-univ.stat 1 0.59 60 $LOG
##                                                 1    2       3           4    5  6   7                 8                  9 10   11 12 
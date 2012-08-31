#!/bin/sh

TESTSDIR=$(dirname $0)
${TESTSDIR}/fuzzcomparerender Padauk my_HeadwordSyllables --passes=1 "$@"
${TESTSDIR}/fuzzcomparerender Annapurnarc2 udhr_nep  --passes=1 "$@" 
${TESTSDIR}/fuzzcomparerender charis_r_gr udhr_yor  --passes=1 "$@"
${TESTSDIR}/fuzzcomparerender Scheherazadegr udhr_arb  --passes=1 -r "@"


